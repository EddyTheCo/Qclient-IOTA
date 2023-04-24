#include"client/qclient.hpp"

#include <QCoreApplication>



using namespace qiota::qblocks;
using namespace qiota;

int main(int argc, char** argv)
{

    QCoreApplication a(argc, argv);
    auto iota_client=new Client();
    iota_client->set_node_address(QUrl(argv[1]));
    //https://api.testnet.shimmer.network
    //ef4593558d0c3ed9e3f7a2de766d33093cd72372c800fa47ab5765c43ca006b5

    if(argc>1)iota_client->set_jwt(argv[2]);


    const auto data_=dataF("WENN?, SOON");
    const auto tag_=tagF("from Iota-Qt");
    const auto payload_=Payload::Tagged_Data(tag_,data_);

    auto block_=Block(payload_);

    iota_client->send_block(block_);

    return a.exec();
}
