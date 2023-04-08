#include"client/qclient.hpp"
#include"crypto/qed25519.hpp"
#include"block/qblock.hpp"
#include"crypto/qslip10.hpp"
#include"qaddr_bundle.hpp"
#include<iostream>
#include <QCoreApplication>
#include <QCryptographicHash>
#include<QJsonDocument>
#undef NDEBUG
#include <assert.h>


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
                    const auto aliasid=addr_bundle->alias_outputs.front()->get_id();
                    const auto ailasaddress=std::shared_ptr<qblocks::Address>(new Alias_Address(aliasid));
                    auto alias_bundle= new address_bundle(ailasaddress);
                    qDebug()<<"aliasAddress="<<alias_bundle->get_address_bech32(info->bech32Hrp);
                    auto foundry_node_outputs_=new Node_outputs();
                    QObject::connect(foundry_node_outputs_,&Node_outputs::finished,iota_client,[=]( ){


                        alias_bundle->consume_outputs(foundry_node_outputs_->outs_,0);
                        qDebug()<<"foundries:"<<alias_bundle->foundry_outputs.size();
                        if(addr_bundle->amount+alias_bundle->amount)
                        {
                            if(alias_bundle->foundry_outputs.size())
                            {
                                auto eddAddr=addr_bundle->get_address();
                                const auto addUnlock=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(eddAddr));
                                auto stateUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new State_Controller_Address_Unlock_Condition(eddAddr));
                                auto goveUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Governor_Address_Unlock_Condition(eddAddr));

                                addr_bundle->alias_outputs.front()->unlock_conditions_={stateUnlcon,goveUnlcon};


                                auto aliasUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Immutable_Alias_Address_Unlock_Condition(ailasaddress));
                                auto aliasoutput=std::dynamic_pointer_cast<qblocks::Alias_Output>(addr_bundle->alias_outputs.front());
                                aliasoutput->state_index_++;
                                auto min_funds_alias=addr_bundle->alias_outputs.front()->
                                        min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);
                                addr_bundle->alias_outputs.front()->amount_=min_funds_alias;

                                alias_bundle->foundry_outputs.front()->amount_=alias_bundle->foundry_outputs.front()->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);

                                for(const auto& v:addr_bundle->get_tokens())
                                {
                                    alias_bundle->native_tokens[v->token_id()]+=v->amount();
                                }

                                auto BaOut= std::shared_ptr<qblocks::Output>
                                        (new Basic_Output(addr_bundle->amount+alias_bundle->amount-
                                                          addr_bundle->alias_outputs.front()->amount_-
                                                          alias_bundle->foundry_outputs.front()->amount_,{addUnlock},{},alias_bundle->get_tokens()));
                                if(addr_bundle->amount+alias_bundle->amount>=                                                          addr_bundle->alias_outputs.front()->amount_-
                                        alias_bundle->foundry_outputs.front()->amount_+
                                        alias_bundle->foundry_outputs.front()->amount_+BaOut->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost))
                                {
                                    std::vector<std::shared_ptr<qblocks::Output>> the_outputs_{
                                        addr_bundle->alias_outputs.front(),alias_bundle->foundry_outputs.front(),BaOut};

                                    auto Inputs_Commitment=c_array(QCryptographicHash::hash(addr_bundle->Inputs_Commitments+alias_bundle->Inputs_Commitments, QCryptographicHash::Blake2b_256));
                                    auto inputs=addr_bundle->inputs;
                                    inputs.insert(inputs.end(),alias_bundle->inputs.begin(),alias_bundle->inputs.end());
                                    auto essence=std::shared_ptr<qblocks::Essence>(
                                                new Transaction_Essence(info->network_id_,inputs,Inputs_Commitment,the_outputs_,nullptr));

                                    c_array serializedEssence;
                                    serializedEssence.from_object<Essence>(*essence);

                                    auto essence_hash=QCryptographicHash::hash(serializedEssence, QCryptographicHash::Blake2b_256);

                                    addr_bundle->create_unlocks(essence_hash);
                                    auto unlocks=addr_bundle->unlocks;
                                    alias_bundle->create_unlocks(essence_hash);
                                    unlocks.insert(unlocks.end(),alias_bundle->unlocks.begin(),alias_bundle->unlocks.end());
                                    auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,unlocks));
                                    auto block_=Block(trpay);
                                    iota_client->send_block(block_);
                                }
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
