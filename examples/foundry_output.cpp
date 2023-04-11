#include"client/qclient.hpp"
#include"crypto/qed25519.hpp"
#include"crypto/qslip10.hpp"
#include"qaddr_bundle.hpp"

#include <QCoreApplication>

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

    QJsonObject metadatajson;
    metadatajson.insert("standard","IRC30");
    metadatajson.insert("name","FooCoin");
    metadatajson.insert("symbol","FOO");

    auto  info=iota_client->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,a,[=]( ){
        auto addr_bundle=new AddressBundle(qed25519::create_keypair(keys.secret_key()));
        const auto address=addr_bundle->get_address_bech32(info->bech32Hrp);
        qDebug()<<"address:"<<address;
        auto node_outputs_=new Node_outputs();

        QObject::connect(node_outputs_,&Node_outputs::finished,iota_client,[=]( ){

            auto alias_node_outputs_=new Node_outputs();
            QObject::connect(alias_node_outputs_,&Node_outputs::finished,iota_client,[=]( ){

                auto eddAddr=addr_bundle->get_address();

                //********* Consume all the basic oututs of the address ********************//
                addr_bundle->consume_outputs(node_outputs_->outs_,0);

                //********* Consume all the alias oututs of the address ********************//
                addr_bundle->consume_outputs(alias_node_outputs_->outs_,0);

                if(addr_bundle->alias_outputs.size())
                {

                    //********* Get the first alias output from the consumed outputs ********************//
                    auto aliasOut=addr_bundle->alias_outputs.front();

                    const auto addUnlock=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(eddAddr));
                    auto stateUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new State_Controller_Address_Unlock_Condition(eddAddr));
                    auto goveUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Governor_Address_Unlock_Condition(eddAddr));

                    //********* Reset the unlock conditions of the alias output ********************//
                    aliasOut->unlock_conditions_={stateUnlcon,goveUnlcon};

                    //********* State transition the alias output ********************//
                    auto aliasoutput=std::dynamic_pointer_cast<qblocks::Alias_Output>(addr_bundle->alias_outputs.front());
                    aliasoutput->state_index_++;

                    //********* Add 1 to the foundry counter because we will create a foundry ********************//
                    aliasoutput->foundry_counter_++;

                    //********* Set the amount to the minimum storage deposit ********************//
                    aliasOut->amount_=Client::get_deposit(aliasOut,info);

                    //********* Create foundry output ********************//
                    const auto serial_number=aliasoutput->foundry_counter_;

                    auto minted_tokens=quint256(100000000000);
                    auto melted_tokens=quint256();
                    auto maximum_supply=quint256(100000000000);
                    maximum_supply*=1000000;

                    const auto token_scheme=std::shared_ptr<qblocks::Token_Scheme>
                            (new Simple_Token_Scheme(minted_tokens,melted_tokens,maximum_supply));

                    auto metadata=QJsonDocument(metadatajson).toJson(QJsonDocument::Indented);
                    auto immetFea=std::shared_ptr<qblocks::Feature>(new Metadata_Feature(fl_array<quint16>(metadata)));
                    auto metFea=std::shared_ptr<qblocks::Feature>(new Metadata_Feature(fl_array<quint16>("Iota-Qt")));

                    const auto ailasaddress=std::shared_ptr<qblocks::Address>(new Alias_Address(aliasOut->get_id()));
                    auto aliasUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Immutable_Alias_Address_Unlock_Condition(ailasaddress));

                    //********* Create the foundry output with all the amount ofthe consumed outputs minus alias output deposit storage ********************//
                    auto foundOut= std::shared_ptr<qblocks::Output>
                            (new Foundry_Output(addr_bundle->amount-aliasOut->amount_,{aliasUnlcon},token_scheme,serial_number,{metFea},{},{immetFea}));
                    auto tokenid=foundOut->get_id();

                    //********* Create Native token from the foundry to put on output ********************//
                    auto nativeToken=std::shared_ptr<Native_Token>(new Native_Token(tokenid,minted_tokens));

                    //********* Add the new token to the list of tokens from consumed outputs ********************//
                    addr_bundle->native_tokens[nativeToken->token_id()]+=nativeToken->amount();

                    //********* The foundry output will contain now all the tokens ********************//
                    foundOut->native_tokens_=addr_bundle->get_tokens();

                    //********* Check we have enough to cover the storage deposit ********************//
                    if(addr_bundle->amount>=aliasOut->amount_+Client::get_deposit(foundOut,info))
                    {

                        std::vector<std::shared_ptr<qblocks::Output>> the_outputs_{addr_bundle->alias_outputs.front(),foundOut};
                        the_outputs_.insert(the_outputs_.end(),addr_bundle->ret_outputs.begin(),addr_bundle->ret_outputs.end());

                        auto Inputs_Commitment=Block::get_inputs_Commitment(addr_bundle->Inputs_hash);

                        auto essence=std::shared_ptr<qblocks::Essence>(
                                    new Transaction_Essence(info->network_id_,addr_bundle->inputs,Inputs_Commitment,the_outputs_,nullptr));

                        addr_bundle->create_unlocks(essence->get_hash());
                        auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,addr_bundle->unlocks));
                        auto block_=Block(trpay);
                        iota_client->send_block(block_);
                    }

                }
                alias_node_outputs_->deleteLater();
                node_outputs_->deleteLater();
                info->deleteLater();

            });
            iota_client->get_outputs<qblocks::Output::Alias_typ>(alias_node_outputs_,"stateController="+address);
        });
        iota_client->get_outputs<qblocks::Output::Basic_typ>(node_outputs_,"address="+address);
    });


    return a->exec();

}
