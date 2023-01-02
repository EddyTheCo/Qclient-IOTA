#pragma once

#include"client/qnode_response.hpp"
#include<QByteArray>
#include <QJsonValue>
namespace qiota{


class Node_info : public QObject
{
    Q_OBJECT
public:
    Node_info(Response*);
    quint64 network_id_;
    quint8  protocol_version;
    quint32 min_pow_score;
    QString bech32Hrp;
    quint64 vByteFactorKey,vByteFactorData,vByteCost;
public slots:
    void fill(QJsonValue data);
signals:
    void finished(void);
private:
    Response* response_;
    QByteArray network_name_;
};


};
