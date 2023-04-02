#include"client/qnode_block.hpp"
#include<QJsonObject>
#include<QJsonDocument>
#include<QDebug>
namespace qiota{

Node_block::Node_block(Response* resp):response_(resp)
{
    QObject::connect(resp, &Response::returned,this, &Node_block::fill);
}
void Node_block::fill(QJsonValue data)
{
    block_=qblocks::Block(data);
    emit finished();
    response_->deleteLater();
}
qblocks::c_array Node_block::ready(void)const
{
    qblocks::c_array serialblock;
    serialblock.from_object(block_);
    serialblock.resize(serialblock.size()-8); //remove nonce
    return serialblock;
}
void Node_block::set_pv(const quint8& pv){
    block_.set_pv(pv);
}
void Node_block::set_parents(const std::vector<qblocks::Block_ID>& parents_m)
{
    block_.set_parents(parents_m);
    qDebug().noquote()<<"block_.set_parents:\n"<<QString(QJsonDocument(block_.get_Json()).toJson(QJsonDocument::Indented));
}
void Node_block::Node_block::set_nonce(const quint64& nonce_m){

    block_.set_nonce(nonce_m);
    qDebug().noquote()<<"block_.set_nonce:\n"<<QString(QJsonDocument(block_.get_Json()).toJson(QJsonDocument::Indented));
    emit finished();
};
}
