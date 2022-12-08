#pragma once

#include"block/qblock.hpp"
#include"client/qnode_response.hpp"
#include <QJsonValue>
namespace qiota{


class Node_tips : public QObject
{
    Q_OBJECT
public:
    Node_tips(Response*);
    std::vector<qblocks::block_id> tips;
public slots:
    void fill(QJsonValue data);
signals:
    void finished(void);

private:
    Response* response_;

};

};
