#include"client/qnode_response.hpp"
#include<QJsonDocument>
#include<QJsonObject>
namespace qiota{

Response::Response(QNetworkReply *thereply,QObject *parent):QObject(parent),reply(thereply)
{
    QObject::connect(reply, &QNetworkReply::finished,this, &Response::fill);
    QObject::connect(reply, &QNetworkReply::errorOccurred,this, &Response::error_found);
}
void Response::fill()
{
    if(!reply->error())
    {
        QByteArray response_data = reply->readAll();
        auto data = (QJsonDocument::fromJson(response_data)).object();
        emit returned(data);
        reply->deleteLater();
    }
    else
    {
        reply->deleteLater();
    }

}
void Response::error_found(QNetworkReply::NetworkError code)
{
    auto errorreply=reply->errorString();
    qDebug()<<"Error:"<<errorreply;
    qDebug()<<"code:"<<code;
    qDebug()<<"errorfound"<<reply->readAll();
    reply->deleteLater();
}


}
