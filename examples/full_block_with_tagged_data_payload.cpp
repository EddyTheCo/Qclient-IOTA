#include"client/qclient.hpp"

#include <QCoreApplication>
#include <QTimer>


using namespace qiota::qblocks;
using namespace qiota;

int main(int argc, char** argv)
{

    QCoreApplication a(argc, argv);
    auto iota_client=new Client();
    iota_client->set_node_address(QUrl(argv[1]));
    //https://api.testnet.shimmer.network

    if(argc>1)iota_client->set_jwt(argv[2]);


    const auto data_=dataF("WENN?, SOON");
    const auto tag_=tagF("from Iota-Qt");
    const auto payload_=Payload::Tagged_Data(tag_,data_);

    auto block_=Block(payload_);

    iota_client->send_block(block_);

    QObject::connect(iota_client,&Client::last_blockid,&a,[&a](const c_array bid )
    {
        qDebug()<<"blockid:"<<bid.toHexString();
        a.quit();
    });
    QTimer::singleShot(10000, &a, QCoreApplication::quit);
    return a.exec();
}
