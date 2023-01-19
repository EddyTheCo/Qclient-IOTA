#include"client/qclient.hpp"
#include<iostream>
#include <QCoreApplication>
#undef NDEBUG
#include <assert.h>


using namespace qiota::qblocks;
using namespace qiota;

int main(int argc, char** argv)
{

    QCoreApplication a(argc, argv);
    auto iota=new Client(QUrl(argv[1]));

    auto data_=dataF("hello form testing");
    auto tag_=tagF("testing from hello");
    auto payload_=std::shared_ptr<Payload>(new Tagged_Data_Payload(tag_,data_));
    auto block_=Block(payload_);

    iota->send_block(block_);

    auto info=iota->get_api_core_v2_info();


    return a.exec();
}
