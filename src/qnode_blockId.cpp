#include "client/qnode_blockId.hpp"
#include <QDebug>
#include <QJsonObject>
namespace qiota
{

Node_blockID::Node_blockID(Response *resp) : response_(resp)
{
    QObject::connect(resp, &Response::returned, this, &Node_blockID::fill);
}
void Node_blockID::fill(QJsonValue data)
{
    id = qblocks::Block_ID(data["blockId"]);
    emit finished();
    response_->deleteLater();
}
} // namespace qiota
