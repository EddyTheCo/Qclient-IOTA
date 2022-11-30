#include"client/qclient.hpp"
#include"pow/qpow.hpp"
#include"block/qblock.hpp"
#include<iostream>
#include <QCoreApplication>
#undef NDEBUG
#include <assert.h>


using namespace qiota::qblocks;
using namespace qiota::qpow;
using namespace qiota;

int main(int argc, char** argv)
{

    QCoreApplication a(argc, argv);
    auto iota=new Client(QUrl("https://api.testnet.shimmer.network"));






    auto data_=dataF("hello world");
    auto tag_=tagF("IOTA");
    auto payload_=Tagged_Data_Payload(tag_,data_);
    auto block_=new Block(&payload_);
    qDebug()<<"block:"<<block_->get_Json();
    auto nfinder=new nonceFinder();

    auto node_info=iota->get_api_core_v2_info();
    QObject::connect(node_info,&Response::returned,[=](QJsonValue data ){
    auto pv=(data["protocol"].toObject())["version"].toInt();
    auto minPowScore=(data["protocol"].toObject())["minPowScore"].toInt();
    qDebug()<<"pv:"<<pv<<" minpowScore:"<<minPowScore;
    block_->set_pv(pv);
    qDebug()<<"block:"<<block_->get_Json();
    auto parents=iota->get_api_core_v2_tips();
    QObject::connect(parents,&Response::returned,[=](QJsonValue data ){
    auto tips=(data["tips"].toArray());
    qDebug()<<"tips:"<<tips;
    std::vector<block_id> parents;
    for(auto v:tips)parents.push_back(block_id(v));
    block_->set_parents(parents);
    qDebug()<<"block:"<<block_->get_Json();

    c_array serialized_block;
    (*serialized_block.get_buffer())<(*block_);

    nfinder->calculate(serialized_block,minPowScore);

    QObject::connect(nfinder,&nonceFinder::nonce_found,[=](const quint64 &s){
        qDebug()<<"nonce:"<<s;
       block_->set_nonce(s);
       qDebug()<<"block:"<<block_->get_Json();
       auto res=iota->post_api_core_v2_blocks(block_->get_Json());
       QObject::connect(res,&Response::returned,[](QJsonValue data ){
           qDebug()<<"data:"<<data;
       });

    });
    });
    });



    return a.exec();
}
