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
    metadatajson.insert("standard","IRC27");
    metadatajson.insert("version","v1.0");
    metadatajson.insert("type","image/png");
    metadatajson.insert("uri","https://eddytheco.github.io/profpic.png");
    metadatajson.insert("name","profile picture");

    auto info=iota_client->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,a,[=]( ){
        auto addr_bundle=new AddressBundle(qed25519::create_keypair(keys.secret_key()));
        const auto address=addr_bundle->get_address_bech32<Address::Ed25519_typ>(info->bech32Hrp);
        qDebug()<<"address:"<<address;
        auto node_outputs_=new Node_outputs();
        iota_client->get_basic_outputs(node_outputs_,"address="+address);

        QObject::connect(node_outputs_,&Node_outputs::finished,iota_client,[=]( ){

            auto eddAddr=std::shared_ptr<Address>(new Ed25519_Address(addr_bundle->get_hash()));
            auto issuerFea=std::shared_ptr<qblocks::Feature>(new Issuer_Feature(eddAddr));
            auto metadata=QJsonDocument(metadatajson).toJson(QJsonDocument::Indented);
            auto metFea=std::shared_ptr<qblocks::Feature>(new Metadata_Feature(fl_array<quint16>(metadata)));


            auto addUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(eddAddr));

            auto NftOut= std::shared_ptr<qblocks::Output>(new NFT_Output(0,{addUnlcon},{},{},{issuerFea,metFea}));


            const auto stora_deposit=NftOut->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);
            qDebug()<<"stora_deposit:"<<stora_deposit;
            c_array Inputs_Commitments;
            quint64 amount=0;
            std::vector<std::shared_ptr<qblocks::Output>> ret_outputs;
            std::vector<std::shared_ptr<qblocks::Input>> inputs;

            addr_bundle->consume_outputs(node_outputs_->outs_,stora_deposit,Inputs_Commitments,amount,ret_outputs,inputs);
            if(amount>=stora_deposit)
            {
                qDebug()<<"amount:"<<amount;
                auto BaOut=std::shared_ptr<qblocks::Output>(new Basic_Output(0,{addUnlcon},{},{}));
                if(amount>stora_deposit)
                {

                    const auto min_deposit=BaOut->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);
                    qDebug()<<"min_deposit:"<<min_deposit;
                    if(min_deposit>amount-stora_deposit)
                    {
                        quint64 baamount=0;
                        addr_bundle->consume_outputs(
                        node_outputs_->outs_,min_deposit-(amount-stora_deposit),Inputs_Commitments,baamount,ret_outputs,inputs );
                        if(baamount>=min_deposit-(amount-stora_deposit))BaOut->amount_=baamount+amount-stora_deposit;
                        qDebug()<<"baamount:"<<baamount;
                    }
                    else
                    {
                        BaOut->amount_=amount-stora_deposit;
                    }

                }
                NftOut->amount_=stora_deposit;
                std::vector<std::shared_ptr<qblocks::Output>> the_outputs_{NftOut};
                if(BaOut->amount_)the_outputs_.push_back(BaOut);
                the_outputs_.insert(the_outputs_.end(), ret_outputs.begin(), ret_outputs.end());
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
