#include"client/qclient.hpp"
#include<QJsonDocument>
#include<QJsonObject>
#include<iostream>
namespace qiota{

Response::Response(QNetworkReply *thereply):reply(thereply)
{
    QObject::connect(reply, &QNetworkReply::finished,this, &Response::fill);
    QObject::connect(reply, &QNetworkReply::errorOccurred,this, &Response::error_found);
}
void Response::fill()
{
    QByteArray response_data = reply->readAll();
    auto data = (QJsonDocument::fromJson(response_data)).object();
    emit returned(data);
    reply->deleteLater();
}
void Response::error_found(QNetworkReply::NetworkError code)
{
    auto errorreply=reply->errorString();
    qDebug()<<"Error:"<<errorreply;
}

Client::Client(const QUrl& rest_node_address):
    rest_node_address_(rest_node_address),nam(new QNetworkAccessManager())
{};
Response*  Client::get_reply_rest(const QString& path,const QString& query)const
{
    QUrl InfoUrl=rest_node_address_;
    InfoUrl.setPath(path);
    InfoUrl.setQuery(query);
    auto request=QNetworkRequest(InfoUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return new Response(nam->get(request));
}
Response*  Client::post_reply_rest(const QString& path, const QJsonObject& payload)const
{
    QUrl InfoUrl=rest_node_address_;
    InfoUrl.setPath(path);
    auto request=QNetworkRequest(InfoUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return new Response(nam->post(request,QJsonDocument(payload).toJson()));
}
Response* Client::get_api_core_v2_info()const
{
    return get_reply_rest("/api/core/v2/info");
}
Response* Client::get_api_core_v2_tips()const
{
    return get_reply_rest("/api/core/v2/tips");
}
Response* Client::post_api_core_v2_blocks(const QJsonObject& block_)const
{
    return post_reply_rest("/api/core/v2/blocks",block_);
}
Response* Client::get_api_core_v2_blocks_blockId(const QString& blockId)const
{
    return get_reply_rest("/api/core/v2/blocks/"+blockId);
}
Response* Client::get_api_core_v2_blocks_blockId_metadata(const QString& blockId)const
{
    return get_reply_rest("/api/core/v2/blocks/"+blockId+"/metadata");
}
Response* Client::get_api_core_v2_outputs_outputId(const QString& outputId)const
{
    return get_reply_rest("/api/core/v2/outputs/"+outputId);
}
Response* Client::get_api_core_v2_outputs_outputId_metadata(const QString& outputId)const
{
    return get_reply_rest("/api/core/v2/outputs/"+outputId+"/metadata");
}
Response* Client::get_api_indexer_v1_outputs_basic(const QString& filter)const
{
    return get_reply_rest("/api/indexer/v1/outputs/basic",filter);
}
}
