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
    auto iota_client=new Client(QUrl(argv[1]));
    //https://api.testnet.shimmer.network
    //ef4593558d0c3ed9e3f7a2de766d33093cd72372c800fa47ab5765c43ca006b5
    auto seed=QByteArray::fromHex(argv[2]);

    QVector<quint32> path={44,4219,0,0,0};
    auto MK=Master_key(seed);
    auto keys=MK.slip10_key_from_path(path);

    auto addr_bundle=new AddressBundle(qed25519::create_keypair(keys.secret_key()));

    auto info=iota_client->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,a,[=]( ){

        const auto address=addr_bundle->get_address<Address::Ed25519_typ>();
        qDebug()<<"address:"<<address;
        auto node_outputs_=new Node_outputs();
        iota_client->get_basic_outputs(node_outputs_,"address="+address);

        QObject::connect(node_outputs_,&Node_outputs::finished,iota_client,[=]( ){

            c_array Inputs_Commitments;
            quint64 amount=0;
            std::vector<std::shared_ptr<qblocks::Output>> ret_outputs;
            std::vector<std::shared_ptr<qblocks::Input>> inputs;

            addr_bundle->consume_outputs(node_outputs_->outs_,0,Inputs_Commitments,amount,ret_outputs,inputs);
            if(amount)
            {
                auto eddAddr=std::shared_ptr<Address>(new Ed25519_Address(addr_bundle->get_hash()));
                auto sendFea=std::shared_ptr<qblocks::Feature>(new Sender_Feature(eddAddr));
                auto tagFea=std::shared_ptr<qblocks::Feature>(new Tag_Feature(fl_array<quint8>("tag from IOTA-QT")));
                auto metFea=std::shared_ptr<qblocks::Feature>(new Metadata_Feature(fl_array<quint16>("data from IOTA-QT")));

                auto addUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(eddAddr));
                auto BaOut= std::shared_ptr<qblocks::Output>(new Basic_Output(amount,{addUnlcon},{sendFea,tagFea,metFea},{}));
                std::vector<std::shared_ptr<qblocks::Output>> the_outputs_{BaOut};
                the_outputs_.insert( the_outputs_.end(), ret_outputs.begin(), ret_outputs.end());
                auto Inputs_Commitment=c_array(QCryptographicHash::hash(Inputs_Commitments, QCryptographicHash::Blake2b_256));
                auto essence=std::shared_ptr<qblocks::Essence>(
                            new Transaction_Essence(info->network_id_,inputs,Inputs_Commitment,the_outputs_,nullptr));

                c_array serializedEssence;
                serializedEssence.from_object<Essence>(*essence);

                auto essence_hash=QCryptographicHash::hash(serializedEssence, QCryptographicHash::Blake2b_256);
                std::vector<std::shared_ptr<qblocks::Unlock>> unlocks;
                addr_bundle->create_unlocks<qblocks::Reference_Unlock>(essence_hash,unlocks);

                auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,{unlocks}));
                auto block_=Block(trpay);
                iota_client->send_block(block_);

            }
            info->deleteLater();
        });
    });


    return a->exec();
}
