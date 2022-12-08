#include"client/qnode_blockId.hpp"
#include<QJsonObject>
#include<QDebug>
namespace qiota{

Node_blockID::Node_blockID(Response* resp):response_(resp)
{
    QObject::connect(resp, &Response::returned,this, &Node_blockID::fill);
}
void Node_blockID::fill(QJsonValue data)
{
     id=qblocks::block_id(data["blockId"]);
     emit finished();
     response_->deleteLater();
}
}
