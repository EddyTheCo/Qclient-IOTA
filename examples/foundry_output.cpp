#include"client/qclient.hpp"
#include"crypto/qed25519.hpp"
#include"crypto/qslip10.hpp"
#include"qaddr_bundle.hpp"

#include <QCoreApplication>

#include<QJsonDocument>
#include<QTimer>
using namespace qiota::qblocks;
using namespace qiota;
using namespace qcrypto;

int main(int argc, char** argv)
{

    auto a=new QCoreApplication(argc, argv);

    auto iota_client=new Client();
    iota_client->set_node_address(QUrl(argv[1]));
    //Print the block id after sent and close
    QObject::connect(iota_client,&Client::last_blockid,a,[=](const c_array bid )
    {
        qDebug()<<"blockid:"<<bid.toHexString();
        a->quit();
    });
    //Close the application after 30 secs
    QTimer::singleShot(30000, a, QCoreApplication::quit);


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

                    const auto addUnlock=Unlock_Condition::Address(eddAddr);

                    const auto stateUnlcon=Unlock_Condition::State_Controller_Address(eddAddr);

                    const auto goveUnlcon=Unlock_Condition::Governor_Address(eddAddr);


                    //********* Reset the unlock conditions of the alias output ********************//
                    aliasOut->unlock_conditions_={stateUnlcon,goveUnlcon};

                    //********* State transition the alias output ********************//
                    auto aliasoutput=std::static_pointer_cast<Alias_Output>(aliasOut);
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

                    const auto token_scheme=Token_Scheme::Simple(minted_tokens,melted_tokens,maximum_supply);

                    const auto metadata=QJsonDocument(metadatajson).toJson(QJsonDocument::Indented);
                    const auto immetFea=Feature::Metadata(metadata);

                    const auto metFea=Feature::Metadata("WENN? SOON");

                    const auto ailasaddress=Address::Alias(aliasOut->get_id());

                    const auto aliasUnlcon=Unlock_Condition::Immutable_Alias_Address(ailasaddress);

                    //********* Create the foundry output with all the amount ofthe consumed outputs minus alias output deposit storage ********************//
                    auto foundOut= Output::Foundry(addr_bundle->amount-aliasOut->amount_,{aliasUnlcon},
                                                   token_scheme,serial_number,{},{immetFea},{metFea});

                    auto tokenid=foundOut->get_id();

                    //********* Create Native token from the foundry to put on output ********************//
                    auto nativeToken=Native_Token::Native(tokenid,minted_tokens);

                    //********* Add the new token to the list of tokens from consumed outputs ********************//
                    addr_bundle->native_tokens[nativeToken->token_id()]+=nativeToken->amount();

                    //********* The foundry output will contain now all the tokens ********************//
                    foundOut->native_tokens_=addr_bundle->get_tokens();

                    //********* Check we have enough to cover the storage deposit ********************//
                    if(addr_bundle->amount>=aliasOut->amount_+Client::get_deposit(foundOut,info))
                    {

                        pvector<const Output> the_outputs_{aliasOut,foundOut};
                        the_outputs_.insert(the_outputs_.end(),addr_bundle->ret_outputs.begin(),addr_bundle->ret_outputs.end());

                        auto Inputs_Commitment=Block::get_inputs_Commitment(addr_bundle->Inputs_hash);

                        auto essence=Essence::Transaction(info->network_id_,addr_bundle->inputs,Inputs_Commitment,the_outputs_);

                        addr_bundle->create_unlocks(essence->get_hash());
                        auto trpay=Payload::Transaction(essence,addr_bundle->unlocks);
                        auto block_=Block(trpay);
                        iota_client->send_block(block_);
                    }

                }
                alias_node_outputs_->deleteLater();
                node_outputs_->deleteLater();
                info->deleteLater();

            });
            iota_client->get_outputs<Output::Alias_typ>(alias_node_outputs_,"stateController="+address);
        });
        iota_client->get_outputs<Output::Basic_typ>(node_outputs_,"address="+address);
    });

    return a->exec();

}
