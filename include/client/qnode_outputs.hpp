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
private:
    std::shared_ptr<qblocks::Output> out_;
    qblocks::Output_Metadata_Response metadata_;
};

class Node_outputs : public QObject
{
    Q_OBJECT
public:
    Node_outputs(void);
    std::vector<std::shared_ptr<qblocks::Output>> outs_;
    std::vector<qblocks::transaction_id> transids_;
    std::vector<quint16> outputIndexs_;
    size_t size_;

public slots:
    void fill(QJsonValue data);
signals:
    void finished(void);


};

};
