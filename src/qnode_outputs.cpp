#include "client/qnode_outputs.hpp"
#include <QDebug>
#include <QJsonObject>
namespace qiota
{

Node_outputs::Node_outputs(QObject *parent) : QObject(parent), size_(0)
{
}

Node_output::Node_output(QJsonValue data)
    : metadata_(data["metadata"]), out_(qblocks::Output::from_<const QJsonValue>(data["output"]))
{
}
void Node_outputs::fill(QJsonValue data)
{
    outs_.push_back(Node_output(data));

    if (outs_.size() == size_)
    {
        emit finished();
    }
}
} // namespace qiota
