#include "client/qnode_tips.hpp"
#include <QJsonObject>
namespace qiota
{

Node_tips::Node_tips(Response *resp) : response_(resp)
{
    QObject::connect(resp, &Response::returned, this, &Node_tips::fill);
}
void Node_tips::fill(QJsonValue data)
{
    auto tips_arr = (data["tips"].toArray());
    for (auto v : tips_arr)
        tips.push_back(qblocks::Block_ID(v));
    emit finished();
    response_->deleteLater();
}
} // namespace qiota
