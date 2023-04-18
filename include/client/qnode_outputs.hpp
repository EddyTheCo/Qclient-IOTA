#pragma once

#include"block/qblock.hpp"
#include"client/qnode_response.hpp"
#include <QJsonValue>
namespace qiota{

class Node_output
{
public:
    Node_output(QJsonValue data);
    std::shared_ptr<qblocks::Output> output(void)const{return out_;}
    qblocks::Output_Metadata_Response metadata(void)const{return metadata_;}
    QJsonObject get_Json()const
    {
        QJsonObject var;
        var.insert("metadata",metadata_.get_Json());
        var.insert("output",out_->get_Json());
        return var;
    };
private:
    std::shared_ptr<qblocks::Output> out_;
    qblocks::Output_Metadata_Response metadata_;
};

class Node_outputs : public QObject
{
    Q_OBJECT
public:
    Node_outputs(QObject *parent = nullptr);
    std::vector<Node_output> outs_;
    size_t size_;

public slots:
    void fill(QJsonValue data);
    void fill(){emit finished();};
signals:
    void finished();


};

};
