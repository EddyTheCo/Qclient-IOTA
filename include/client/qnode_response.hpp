#pragma once

#include <QNetworkReply>
#include <QJsonValue>
namespace qiota{


class Response: public QObject
{
    Q_OBJECT
public:
    Response(QNetworkReply *thereply);
public slots:
    void fill();
    void error_found(QNetworkReply::NetworkError code);
signals:
    void returned( QJsonValue data );
private:
    QNetworkReply *reply;

};



};
