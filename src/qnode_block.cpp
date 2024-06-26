#include "client/qnode_block.hpp"
#include <QDebug>
#include <QJsonObject>
namespace qiota
{

Node_block::Node_block(Response *resp) : response_(resp)
{
    QObject::connect(resp, &Response::returned, this, &Node_block::fill);
}
void Node_block::fill(QJsonValue data)
{
    block_ = qblocks::Block(data);
    emit finished();
    response_->deleteLater();
}
qblocks::c_array Node_block::ready(void) const
{
    qblocks::c_array serialblock;
    serialblock.from_object(block_);
    serialblock.resize(serialblock.size() - 8); // remove nonce
    return serialblock;
}
void Node_block::set_pv(const quint8 &pv)
{
    block_.set_pv(pv);
}
void Node_block::set_parents(const std::vector<qblocks::Block_ID> &parents_m)
{
    block_.set_parents(parents_m);
}
void Node_block::Node_block::set_nonce(const quint64 &nonce_m)
{

    block_.set_nonce(nonce_m);
    emit finished();
};
} // namespace qiota
