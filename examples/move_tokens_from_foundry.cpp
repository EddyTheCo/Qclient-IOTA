#include"client/qclient.hpp"
#include"crypto/qed25519.hpp"
#include"crypto/qslip10.hpp"
#include"qaddr_bundle.hpp"

#include <QCoreApplication>



using namespace qiota::qblocks;
using namespace qiota;
using namespace qcrypto;

int main(int argc, char** argv)
{

    auto a=new QCoreApplication(argc, argv);

    auto iota_client=new Client();
    iota_client->set_node_address(QUrl(argv[1]));
    //https://api.testnet.shimmer.network
    //ef4593558d0c3ed9e3f7a2de766d33093cd72372c800fa47ab5765c43ca006b5
    auto seed=QByteArray::fromHex(argv[2]);

    if(argc>2)iota_client->set_jwt(argv[3]);

    QVector<quint32> path={44,4219,0,0,0};
    auto MK=Master_key(seed);
    auto keys=MK.slip10_key_from_path(path);


    auto  info=iota_client->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,a,[=]( ){
        auto addr_bundle=new AddressBundle(qed25519::create_keypair(keys.secret_key()));
        const auto address=addr_bundle->get_address_bech32(info->bech32Hrp);
        qDebug()<<"address:"<<address;
        auto node_outputs_=new Node_outputs();

        QObject::connect(node_outputs_,&Node_outputs::finished,iota_client,[=]( ){
            auto alias_node_outputs_=new Node_outputs();
            QObject::connect(alias_node_outputs_,&Node_outputs::finished,iota_client,[=]( ){
                addr_bundle->consume_outputs(node_outputs_->outs_,0);
                addr_bundle->consume_outputs(alias_node_outputs_->outs_,0);
                qDebug()<<"alias:"<<addr_bundle->alias_outputs.size();
                if(addr_bundle->alias_outputs.size())
                {
                    auto aliasOut=addr_bundle->alias_outputs.front();

                    const auto ailasaddress=std::shared_ptr<qblocks::Address>(new Alias_Address(aliasOut->get_id()));
                    auto alias_bundle= new address_bundle(ailasaddress);
                    qDebug()<<"aliasAddress="<<alias_bundle->get_address_bech32(info->bech32Hrp);
                    auto foundry_node_outputs_=new Node_outputs();
                    QObject::connect(foundry_node_outputs_,&Node_outputs::finished,iota_client,[=]( ){

                        alias_bundle->consume_outputs(foundry_node_outputs_->outs_,0);
                        qDebug()<<"foundries:"<<alias_bundle->foundry_outputs.size();

                        if(alias_bundle->foundry_outputs.size())
                        {

                            auto eddAddr=addr_bundle->get_address();
                            auto addUnlock=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(eddAddr));
                            auto stateUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new State_Controller_Address_Unlock_Condition(eddAddr));
                            auto goveUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Governor_Address_Unlock_Condition(eddAddr));

                            aliasOut->unlock_conditions_ = {stateUnlcon,goveUnlcon};

                            auto aliasoutput = std::dynamic_pointer_cast<qblocks::Alias_Output>(aliasOut);
                            aliasoutput->state_index_++;

                            aliasOut->amount_ = Client::get_deposit(aliasOut,info);
                            quint64 amount_foundries=0;
                            for(auto & v:alias_bundle->foundry_outputs)
                            {
                                v->amount_=Client::get_deposit(v,info);
                                amount_foundries+=v->amount_;
                            }

                            for(const auto& v:addr_bundle->get_tokens())
                            {
                                alias_bundle->native_tokens[v->token_id()] += v->amount();
                            }

                            const auto leftovers = addr_bundle->amount+alias_bundle->amount-
                                    aliasOut->amount_-
                                    amount_foundries;

                            auto BaOut = std::shared_ptr<qblocks::Output>
                                    (new Basic_Output(leftovers,{addUnlock},{},alias_bundle->get_tokens()));

                            if(leftovers>=Client::get_deposit(BaOut,info))
                            {

                                std::vector<std::shared_ptr<qblocks::Output>> the_outputs_{
                                    aliasOut,BaOut};
                                the_outputs_.insert(the_outputs_.end(),alias_bundle->foundry_outputs.begin(),alias_bundle->foundry_outputs.end());
                                the_outputs_.insert(the_outputs_.end(),addr_bundle->ret_outputs.begin(),addr_bundle->ret_outputs.end());
                                the_outputs_.insert(the_outputs_.end(),alias_bundle->ret_outputs.begin(),alias_bundle->ret_outputs.end());

                                auto Inputs_Commitment=Block::get_inputs_Commitment(addr_bundle->Inputs_hash+alias_bundle->Inputs_hash);


                                auto inputs=addr_bundle->inputs;
                                inputs.insert(inputs.end(),alias_bundle->inputs.begin(),alias_bundle->inputs.end());

                                auto essence=std::shared_ptr<qblocks::Essence>(
                                            new Transaction_Essence(info->network_id_,inputs,Inputs_Commitment,the_outputs_,nullptr));

                                addr_bundle->create_unlocks(essence->get_hash());
                                auto unlocks=addr_bundle->unlocks;
                                alias_bundle->create_unlocks(essence->get_hash(),unlocks.size()-1);
                                unlocks.insert(unlocks.end(),alias_bundle->unlocks.begin(),alias_bundle->unlocks.end());

                                auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,unlocks));
                                auto block_=Block(trpay);
                                iota_client->send_block(block_);
                            }
                        }

                        node_outputs_->deleteLater();
                        alias_node_outputs_->deleteLater();
                        foundry_node_outputs_->deleteLater();
                        info->deleteLater();
                    });
                    iota_client->get_outputs<qblocks::Output::Foundry_typ>(foundry_node_outputs_,"aliasAddress="+alias_bundle->get_address_bech32(info->bech32Hrp));
                }
            });
            iota_client->get_outputs<qblocks::Output::Alias_typ>(alias_node_outputs_,"stateController="+address);
        });
        iota_client->get_outputs<qblocks::Output::Basic_typ>(node_outputs_,"address="+address);
    });


    return a->exec();

}
