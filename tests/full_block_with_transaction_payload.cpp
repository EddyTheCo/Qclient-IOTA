#include"client/qclient.hpp"
#include"pow/qpow.hpp"
#include"block/qblock.hpp"
#include"encoding/qbech32.hpp"
#include"crypto/qed25519.hpp"
#include"crypto/qslip10.hpp"
#include<iostream>
#include <QCoreApplication>
#include <QCryptographicHash>
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
    QObject::connect(iota,&Client::last_blockid,&a,[=](c_array id){
        qDebug()<<"id:"<<id.toHexString();
    });

    auto seed=QByteArray::fromHex("ef4593558d0c3ed9e3f7a2de766d33093cd72372c800fa47ab5765c43ca006b5");

    QVector<quint32> path={44,4219,0,0,0};
    auto MK=Master_key(seed);
    auto keys=MK.slip10_key_from_path(path);

    const auto edkeys=qed25519::create_keypair(keys.secret_key());
    auto publ=c_array(QCryptographicHash::hash(edkeys.first,QCryptographicHash::Blake2b_256));


    auto pub=publ;
    pub.push_front('\x00');


    auto address=Iota::encode("rms",pub);
    qDebug()<<address;
    auto eddAddr=std::shared_ptr<Address>(new Ed25519_Address(publ));


    auto node_outputs_=new Node_outputs();
    iota->get_basic_outputs(node_outputs_,"address="+address+"&hasStorageDepositReturn=false&hasTimelock=false&hasExpiration=false");

    QObject::connect(node_outputs_,&Node_outputs::finished,iota,[=]( ){
        std::vector<std::shared_ptr<qblocks::Input>> inputs;
        c_array Inputs_Commitments;
        quint64 amount=0;
        for(auto i=0;i<node_outputs_->outs_.size();i++)
        {
            inputs.push_back(std::shared_ptr<qblocks::Input>(new qblocks::UTXO_Input(node_outputs_->transids_[i],
                                                                                  node_outputs_->outputIndexs_[i])));
            c_array prevOutputSer;
            prevOutputSer.from_object<qblocks::Output>(*(node_outputs_->outs_[i]));
            auto Inputs_Commitment1=QCryptographicHash::hash(prevOutputSer, QCryptographicHash::Blake2b_256);
            Inputs_Commitments.append(Inputs_Commitment1);
            amount+=std::dynamic_pointer_cast<qblocks::Basic_Output>(node_outputs_->outs_[i])->amount();
        }
        qDebug()<<"total amount:"<<amount;
        auto Inputs_Commitment=c_array(QCryptographicHash::hash(Inputs_Commitments, QCryptographicHash::Blake2b_256));
        auto sendFea=std::shared_ptr<qblocks::Feature>(new Sender_Feature(eddAddr));
        auto tagFea=std::shared_ptr<qblocks::Feature>(new Tag_Feature(fl_array<quint8>("tag from IOTA-QT")));
        //auto metFea=new Metadata_Feature(fl_array<quint16>("data from IOTA-QT"));

        auto addUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(eddAddr));
        auto BaOut= std::shared_ptr<qblocks::Output>(new Basic_Output(amount,{addUnlcon},{sendFea,tagFea},{}));

        auto info=iota->get_api_core_v2_info();
        QObject::connect(info,&Node_info::finished,iota,[=]( ){
            auto essence=std::shared_ptr<qblocks::Essence>(new Transaction_Essence(info->network_id_,inputs,Inputs_Commitment,{BaOut},nullptr));

            c_array serializedEssence;
            serializedEssence.from_object<Essence>(*essence);

            auto essence_hash=QCryptographicHash::hash(serializedEssence, QCryptographicHash::Blake2b_256);
            const auto sign_t=qed25519::sign(edkeys,essence_hash);

            auto signature_t=std::shared_ptr<qblocks::Signature>(new Ed25519_Signature(public_key(edkeys.first),
                                                   signature(sign_t)));

            auto Sigunlock=std::shared_ptr<qblocks::Unlock>(new Signature_Unlock(signature_t));

            auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,{Sigunlock}));

            auto block_=Block(trpay);
             iota->send_block(block_);

        });





    });




        return a.exec();
    }
