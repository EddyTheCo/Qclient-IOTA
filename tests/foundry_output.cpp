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

    QJsonObject metadatajson;
    metadatajson.insert("standard","IRC30");
    metadatajson.insert("name","FooCoin");
    metadatajson.insert("symbol","FOO");


    auto info=iota_client->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,a,[=]( ){
        auto addr_bundle=new AddressBundle(qed25519::create_keypair(keys.secret_key()));
        const auto address=addr_bundle->get_address_bech32(info->bech32Hrp);
        qDebug()<<"address:"<<address;
        auto node_outputs_=new Node_outputs();


        QObject::connect(node_outputs_,&Node_outputs::finished,iota_client,[=]( ){

            QObject::connect(node_outputs_,&Node_outputs::finished,iota_client,[=]( ){
                auto eddAddr=addr_bundle->get_address();
                addr_bundle->consume_outputs(node_outputs_->outs_,0);
                if(addr_bundle->amount)
                {
                    if(addr_bundle->alias_outputs.size())
                    {
                        const auto aliasid=addr_bundle->alias_outputs.front()->get_id();

                        const auto addUnlock=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(eddAddr));
                        addr_bundle->alias_outputs.front()->unlock_conditions_.push_back(addUnlock);
                        const auto ailasaddress=std::shared_ptr<qblocks::Address>(new Alias_Address(aliasid));
                        auto aliasUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Immutable_Alias_Address_Unlock_Condition(ailasaddress));
                        auto aliasoutput=std::dynamic_pointer_cast<qblocks::Alias_Output>(addr_bundle->alias_outputs.front());
                        aliasoutput->state_index_++;
                        const auto serial_number=aliasoutput->foundry_counter_;
                        auto minted_tokens=quint256();
                        auto melted_tokens=quint256();
                        auto maximum_supply=quint256();
                        minted_tokens+=10000;
                        maximum_supply+=10000;
                        const auto token_scheme=std::shared_ptr<qblocks::Token_Scheme>
                                (new Simple_Token_Scheme(minted_tokens,melted_tokens,maximum_supply));
                        auto metadata=QJsonDocument(metadatajson).toJson(QJsonDocument::Indented);
                        auto immetFea=std::shared_ptr<qblocks::Feature>(new Metadata_Feature(fl_array<quint16>(metadata)));
                        auto metFea=std::shared_ptr<qblocks::Feature>(new Metadata_Feature(fl_array<quint16>("Iota-Qt")));

                        auto foundOut= std::shared_ptr<qblocks::Output>
                                (new Foundry_Output(0,{aliasUnlcon},token_scheme,serial_number,{metFea},{},{immetFea}));
                        foundOut->amount_=foundOut->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);
                        addr_bundle->alias_outputs.front()->amount_=addr_bundle->alias_outputs.front()->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);
                        if(addr_bundle->amount>=foundOut->amount_+addr_bundle->alias_outputs.front()->amount_)
                        {
                            const auto bamount=addr_bundle->amount-foundOut->amount_-addr_bundle->alias_outputs.front()->amount_;
                            auto BaOut=std::shared_ptr<qblocks::Output>(new Basic_Output(bamount,{addUnlock},{},{}));
                            const auto minstorage=BaOut->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);
                            std::vector<std::shared_ptr<qblocks::Output>> the_outputs_{addr_bundle->alias_outputs.front(),foundOut};
                            if(bamount&&bamount<minstorage)
                            {
                                addr_bundle->alias_outputs.front()->amount_+=bamount;
                                addr_bundle->alias_outputs.front()->native_tokens_=addr_bundle->get_tokens();
                            }
                            else
                            {
                                BaOut->amount_=bamount;
                                BaOut->native_tokens_=addr_bundle->get_tokens();
                                the_outputs_.push_back(BaOut);
                            }

                            auto Inputs_Commitment=c_array(QCryptographicHash::hash(addr_bundle->Inputs_Commitments, QCryptographicHash::Blake2b_256));
                            auto essence=std::shared_ptr<qblocks::Essence>(
                                        new Transaction_Essence(info->network_id_,addr_bundle->inputs,Inputs_Commitment,the_outputs_,nullptr));

                            c_array serializedEssence;
                            serializedEssence.from_object<Essence>(*essence);

                            auto essence_hash=QCryptographicHash::hash(serializedEssence, QCryptographicHash::Blake2b_256);
                            addr_bundle->create_unlocks(essence_hash);

                            auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,addr_bundle->unlocks));
                            auto block_=Block(trpay);
                            iota_client->send_block(block_);

                        }
                    }
                }
                node_outputs_->deleteLater();
                info->deleteLater();
            });
             iota_client->get_alias_outputs(node_outputs_,"stateController="+address);
        });
        iota_client->get_basic_outputs(node_outputs_,"address="+address);
    });


    return a->exec();

}
