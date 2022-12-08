#pragma once

#include"block/qblock.hpp"
#include"client/qnode_response.hpp"
#include <QJsonValue>
namespace qiota{


class Node_blockID : public QObject
{
    Q_OBJECT
public:
    Node_blockID(){};
    Node_blockID(Response*);
    qblocks::block_id id;
public slots:
    void fill(QJsonValue data);
signals:
    void finished(void);

private:
    Response* response_;
};

};
