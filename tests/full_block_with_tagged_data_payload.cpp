#include"client/qclient.hpp"
#include"pow/qpow.hpp"
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

    auto data_=dataF("hello");
    auto tag_=tagF("IOTA");
    auto payload_=std::shared_ptr<Payload>(new Tagged_Data_Payload(tag_,data_));
    auto block_=Block(payload_);

for(auto i=0;i<125;i++)
{
    iota->send_block(block_);

    QObject::connect(iota,&Client::last_blockid,&a,[=](c_array id){
        qDebug()<<"id:"<<id.toHexString();
    });

}
    return a.exec();
}
