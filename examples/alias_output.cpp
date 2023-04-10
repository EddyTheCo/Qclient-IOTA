#include"client/qclient.hpp"
#include"crypto/qed25519.hpp"
#include"crypto/qslip10.hpp"
#include"qaddr_bundle.hpp"


#include <QCoreApplication>


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



    auto info=iota_client->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,a,[=]( ){
        auto addr_bundle=new AddressBundle(qed25519::create_keypair(keys.secret_key()));
        const auto address=addr_bundle->get_address_bech32(info->bech32Hrp);
        qDebug()<<"address:"<<address;

        auto node_outputs_=new Node_outputs();
        QObject::connect(node_outputs_,&Node_outputs::finished,iota_client,[=]( ){

            auto eddAddr=addr_bundle->get_address();

            auto addUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(eddAddr));

            //********* Create alias output ********************//
            auto stateUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new State_Controller_Address_Unlock_Condition(eddAddr));
            auto goveUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Governor_Address_Unlock_Condition(eddAddr));

            auto aliasOut= std::shared_ptr<qblocks::Output>(new Alias_Output(0,{stateUnlcon,goveUnlcon}));

            const auto stora_deposit=Client::get_deposit(aliasOut,info);
            aliasOut->amount_=stora_deposit;

            addr_bundle->consume_outputs(node_outputs_->outs_,stora_deposit);

            if(addr_bundle->amount>=stora_deposit)
            {

                auto BaOut=std::shared_ptr<qblocks::Output>(new Basic_Output(0,{addUnlcon},{},addr_bundle->get_tokens()));
                if(addr_bundle->amount>stora_deposit)
                {
                    auto min_deposit=Client::get_deposit(BaOut,info);
                    qDebug()<<"min_deposit:"<<min_deposit;
                    if(min_deposit>addr_bundle->amount-stora_deposit)
                    {
                        addr_bundle->consume_outputs(
                                    node_outputs_->outs_,min_deposit-(addr_bundle->amount-stora_deposit));
                        BaOut->native_tokens_=addr_bundle->get_tokens();
                        min_deposit=Client::get_deposit(BaOut,info);
                        if(addr_bundle->amount>=min_deposit+stora_deposit)BaOut->amount_=addr_bundle->amount-stora_deposit;
                    }
                    else
                    {
                        BaOut->amount_=addr_bundle->amount-stora_deposit;
                    }

                }
                std::vector<std::shared_ptr<qblocks::Output>> the_outputs_{aliasOut};
                if(BaOut->amount_)the_outputs_.push_back(BaOut);
                the_outputs_.insert(the_outputs_.end(),addr_bundle->ret_outputs.begin(),addr_bundle->ret_outputs.end());

                auto Inputs_Commitment=c_array(QCryptographicHash::hash(addr_bundle->Inputs_Commitments, QCryptographicHash::Blake2b_256));
                auto essence=std::shared_ptr<qblocks::Essence>(
                            new Transaction_Essence(info->network_id_,addr_bundle->inputs,Inputs_Commitment,the_outputs_,nullptr));

                addr_bundle->create_unlocks(essence->get_hash());

                auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,addr_bundle->unlocks));
                auto block_=Block(trpay);
                iota_client->send_block(block_);

            }
            info->deleteLater();
            node_outputs_->deleteLater();
        });
        iota_client->get_outputs<qblocks::Output::Basic_typ>(node_outputs_,"address="+address);
    });


    return a->exec();

}