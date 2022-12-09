#include"client/qnode_outputs.hpp"
#include<QJsonObject>
#include<QDebug>
namespace qiota{

Node_outputs::Node_outputs(void):size_(0)
{

}
void Node_outputs::fill(QJsonValue data)
{
if(!data["metadata"].toObject()["isSpent"].toBool())
{
    transids_.push_back(qblocks::transaction_id(data["metadata"].toObject()["transactionId"]));
    outputIndexs_.push_back(data["metadata"].toObject()["outputIndex"].toInt());
    outs_.push_back(qblocks::Output::from_<const QJsonValue>(data["output"]));
}
else
{
    size_--;
}
    if(outs_.size()==size_)emit finished();

}
}
