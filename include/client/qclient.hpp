#pragma once
/**
 *  https://github.com/iotaledger/tips/blob/main/tips/TIP-0025/tip-0025.md
 *
 **/

#include<QByteArray>
#include<QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
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


class Client
{

public:
    Client(const QUrl& rest_node_address=QUrl(""));

    Response*  get_reply_rest(const QString& path, const QString &query="")const;
    Response*  post_reply_rest(const QString& path, const QJsonObject& payload )const;


    Response* get_api_core_v2_info(void)const;
    Response* get_api_core_v2_tips(void)const;
    Response* post_api_core_v2_blocks(const QJsonObject& block_)const;
    Response* get_api_core_v2_blocks_blockId(const QString& blockId)const;
    Response* get_api_core_v2_blocks_blockId_metadata(const QString& blockId)const;

    Response* get_api_core_v2_outputs_outputId(const QString& outputId)const;
    Response* get_api_core_v2_outputs_outputId_metadata(const QString& outputId)const;
    Response* get_api_indexer_v1_outputs_basic(const QString& filter)const;

private:

    QUrl rest_node_address_;
    QNetworkAccessManager* nam;
};

};
