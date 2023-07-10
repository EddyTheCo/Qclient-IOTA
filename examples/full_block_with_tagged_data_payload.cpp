#include"client/qclient.hpp"
#include <QCoreApplication>


#include <QTimer> //included only to kill the app after some time


using namespace qiota::qblocks;  // https://eddytheco.github.io/Qclient-IOTA/namespaceqiota_1_1qblocks.html
using namespace qiota;    //https://eddytheco.github.io/Qclient-IOTA/namespaceqiota.html

int main(int argc, char** argv)
{

    QCoreApplication a(argc, argv);

	
    auto iota_client=new Client(); //Create the client object.
    iota_client->set_node_address(QUrl(argv[1]));  //The node address is the first argument passed by the command line.
    //https://api.testnet.shimmer.network

    if(argc>1)iota_client->set_jwt(argv[2]); //The JSON Web Token is the second command line argument(optional) 
					     //This allows sending blocks to nodes with protected route /api/core/v2/blocks.
					     //https://wiki.iota.org/shimmer/hornet/how_tos/post_installation/#jwt-auth
					     //https://editor.swagger.io/?url=https://raw.githubusercontent.com/iotaledger/tips/main/tips/TIP-0025/core-rest-api.yaml

    const auto data_=dataF("WENN?, SOON"); //Set the data field of the payload.
    const auto tag_=tagF("from Iota-Qt");  //Set the  tag field of the payload.
    const auto payload_=Payload::Tagged_Data(tag_,data_);  //Create the Tagged_Data Payload. 

    auto block_=Block(payload_);  //Create the block with a Tagged_Data Payload inside.

    iota_client->send_block(block_); //The client sends the block to the node using the /api/core/v2/blocks route.

 
    //Print the block-id if the block is accepted by the node.
    //https://github.com/iotaledger/tips/blob/main/tips/TIP-0024/tip-0024.md#block-id
    QObject::connect(iota_client,&Client::last_blockid,&a,[&a](const c_array bid )
    {
        qDebug()<<"blockid:"<<bid.toHexString();
        a.quit();
    });
    //Kill the app after 30 seconds.
    QTimer::singleShot(30000, &a, QCoreApplication::quit);
    return a.exec();
}

