#include"client/qclient.hpp"
#include"pow/qpow.hpp"
#include"block/qblock.hpp"
#include"encoding/qbech32.hpp"
#include"crypto/qed25519.hpp"
#include"crypto/qslip10.hpp"
#include<iostream>
#include <QCoreApplication>
#include<QJsonDocument>
#undef NDEBUG
#include <assert.h>


using namespace qiota::qblocks;
using namespace qiota::qpow;
using namespace qiota;
using namespace qcrypto;
using namespace qencoding::qbech32;

int main(int argc, char** argv)
{

    QCoreApplication a(argc, argv);
    auto iota=new Client(QUrl("https://api.testnet.shimmer.network"));

    auto seed=QByteArray::fromHex("ef4593558d0c3ed9e3f7a2de766d33093cd72372c800fa47ab5765c43ca006b5");

    QVector<quint32> path={44,4219,0,0,0};
    auto MK=Master_key(seed);
    auto keys=MK.slip10_key_from_path(path);

    const auto edkeys=qed25519::create_keypair(keys.secret_key());
    QByteArray publ=QCryptographicHash::hash(edkeys.first,QCryptographicHash::Blake2b_256);

    QVector<quint32> path2={44,4219,0,0,1};
    auto MK2=Master_key(seed);
    auto keys2=MK.slip10_key_from_path(path2);

    const auto edkeys2=qed25519::create_keypair(keys2.secret_key());
    QByteArray publ2=QCryptographicHash::hash(edkeys2.first,QCryptographicHash::Blake2b_256);

    auto pub=publ;
    pub.push_front('\x00');
    qDebug()<<pub.toHex();

    auto address=Iota::encode("rms",pub);
    qDebug()<<address;

    auto node_info=iota->get_api_core_v2_info();

    QObject::connect(node_info,&Response::returned,node_info,[=](QJsonValue data ){

        quint64 networkId;
        auto networkname=QByteArray(data["protocol"].toObject()["networkName"].toString().toLatin1());


        QByteArray networkId_hash=QCryptographicHash::hash(networkname,QCryptographicHash::Blake2b_256);
        networkId_hash.truncate(8);

        auto buffer=QDataStream(&networkId_hash,QIODevice::ReadOnly);
        buffer.setByteOrder(QDataStream::LittleEndian);

        buffer>>networkId;

        auto pv=(data["protocol"].toObject())["version"].toInt();
        auto minPowScore=(data["protocol"].toObject())["minPowScore"].toInt();


        auto outputids=iota->get_api_indexer_v1_outputs_basic("address="+address);


        QObject::connect(outputids,&Response::returned,outputids,[=](QJsonValue data ){
            std::vector<transaction_id> outids;
            auto transid=data["items"].toArray();

            auto output=iota->get_api_core_v2_outputs_outputId(transid[0].toString());

            QObject::connect(output,&Response::returned,output,[=](QJsonValue data ){
                auto transid=transaction_id(data["metadata"].toObject()["transactionId"]);

                quint16 outputIndex=data["metadata"].toObject()["outputIndex"].toInt();

                auto prevOutput=Output::from_Json(data["output"]);

                auto input=new UTXO_Input(transid,outputIndex);
                c_array prevOutputSer;
                prevOutput->serialize(*prevOutputSer.get_buffer());
                auto Inputs_Commitment1=QCryptographicHash::hash(prevOutputSer, QCryptographicHash::Blake2b_256);

                const auto Inputs_Commitment=QCryptographicHash::hash(Inputs_Commitment1, QCryptographicHash::Blake2b_256);

                auto eddAddr=new Ed25519_Address(c_array(publ));

                auto sendFea=new Sender_Feature(eddAddr);
                //auto tagFea=new Tag_Feature(fl_array<quint8>("tag from IOTA-QT"));
                //auto metFea=new Metadata_Feature(fl_array<quint16>("data from IOTA-QT"));
                auto addUnlcon=new Address_Unlock_Condition(eddAddr);

                auto BaOut=new Basic_Output(1000000000,{addUnlcon},{sendFea},{});


                auto essence=new Transaction_Essence(networkId,{input},c_array(Inputs_Commitment),{BaOut},nullptr);

                c_array serializedEssence;
                essence->serialize(*serializedEssence.get_buffer());

                auto essence_hash=QCryptographicHash::hash(serializedEssence, QCryptographicHash::Blake2b_256);
                const auto sign_t=qed25519::sign(edkeys,essence_hash);

                auto signature_t=new Ed25519_Signature(public_key(edkeys.first),
                                                       signature(sign_t));

                auto Sigunlock=new Signature_Unlock(signature_t);

                auto trpay=new Transaction_Payload(essence,{Sigunlock});


                auto block_= new Block(trpay);

                block_->set_pv(pv);

                auto parents=iota->get_api_core_v2_tips();

                QObject::connect(parents,&Response::returned,parents,[=](QJsonValue data ){
                    auto tips=(data["tips"].toArray());

                    std::vector<block_id> parents;
                    for(auto v:tips)parents.push_back(block_id(v));
                    block_->set_parents(parents);



                    c_array serialized_block;
                    (*serialized_block.get_buffer())<(*block_);


                    auto nfinder=new nonceFinder();
                    nfinder->calculate(serialized_block,minPowScore);

                    QObject::connect(nfinder,&nonceFinder::nonce_found,nfinder,[=](const quint64 &s){

                        block_->set_nonce(s);
                        qDebug().noquote()<<"block:\n"<<QString(QJsonDocument(block_->get_Json()).toJson(QJsonDocument::Indented));

                        auto res=iota->post_api_core_v2_blocks(block_->get_Json());



                        QObject::connect(res,&Response::returned,[](QJsonValue data ){
                            qDebug()<<"data:"<<data;
                        });

                    });



                });


            });

        });

});

        return a.exec();
    }
