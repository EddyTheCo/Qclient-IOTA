#pragma once

#include"block/qblock.hpp"
#include"client/qnode_response.hpp"
#include <QJsonValue>
namespace qiota{


class Node_block : public QObject
{
    Q_OBJECT
public:
    Node_block(Response*);
    Node_block(const qblocks::Block &block_m):block_(block_m),pv_set(false),parents_set(false){};
    qblocks::Block block_;
public slots:
    void fill(QJsonValue data);
    void set_pv(const quint8& pv);
    void set_parents(const std::vector<qblocks::block_id>& parents_m);
    void set_nonce(const quint64& nonce_m);
signals:
    void finished(void);
    void ready(qblocks::c_array serial_block);

private:
    bool pv_set,parents_set;
    Response* response_;
};

};
