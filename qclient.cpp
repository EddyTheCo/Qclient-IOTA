#include"client/qclient.hpp"
#include"pow/qpow.hpp"
#include<QJsonDocument>
#include<QJsonObject>
#include<iostream>
namespace qiota{

Client::Client(QObject *parent):QObject(parent),
    nam(new QNetworkAccessManager(this))
{
    connect(this,&Client::last_blockid,this,[=](qblocks::c_array id){
        qDebug()<<id.toHexString();
    });
};
void Client::setNodeAddress(const QUrl& nodeAddress)
{

    if((m_nodeAddress!=nodeAddress||m_state!=Connected)&&nodeAddress.isValid())
    {
        setState(Disconnected);
        m_nodeAddress=nodeAddress;
        auto info=get_api_core_v2_info();
        connect(info,&Node_info::finished,this,[=]( ){

            if(info->isHealthy)
            {
                setState(Connected);
            }
            info->deleteLater();
        });
    }
}

Response*  Client::get_reply_rest(const QString& path,const QString& query)
{
    QUrl InfoUrl=m_nodeAddress;
    InfoUrl.setPath(path);
    if(!query.isNull())InfoUrl.setQuery(query);
    auto request=QNetworkRequest(InfoUrl);
    request.setAttribute(QNetworkRequest::UseCredentialsAttribute,false);
    if(!JWT.isNull())request.setRawHeader(QByteArray("Authorization"),
                                          QByteArray(("Bearer " + JWT).toUtf8()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return new Response(nam->get(request),this);
}
Response*  Client::post_reply_rest(const QString& path, const QJsonObject& payload)
{
    QUrl InfoUrl=m_nodeAddress;
    InfoUrl.setPath(path);
    auto request=QNetworkRequest(InfoUrl);
    request.setAttribute(QNetworkRequest::UseCredentialsAttribute,false);
    if(!JWT.isNull())request.setRawHeader(QByteArray("Authorization"),
                                          QByteArray("Bearer ").append(JWT.toUtf8()));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return new Response(nam->post(request,QJsonDocument(payload).toJson()),this);
}
Node_info* Client::get_api_core_v2_info(void)
{
    return new Node_info(get_reply_rest("/api/core/v2/info"));
}
Node_tips* Client::get_api_core_v2_tips()
{
    return new Node_tips(get_reply_rest("/api/core/v2/tips"));
}
Node_blockID *Client::post_api_core_v2_blocks(const QJsonObject& block_)
{
    return new Node_blockID(post_reply_rest("/api/core/v2/blocks",block_));
}
Node_block* Client::get_api_core_v2_blocks_blockId(const QString& blockId)
{
    return new Node_block(get_reply_rest("/api/core/v2/blocks/"+blockId));
}
void Client::getFundsFromFaucet(const QString& bech32Address, const QUrl & faucetAddress)
{    
    QUrl InfoUrl=faucetAddress;
    InfoUrl.setPath("/api/enqueue");
    auto request=QNetworkRequest(InfoUrl);
    request.setAttribute(QNetworkRequest::UseCredentialsAttribute,false);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QJsonObject payload {{"address", bech32Address},};
    auto resp= new Response(nam->post(request,QJsonDocument(payload).toJson()),this);
    connect(resp,&Response::returned,this,[=](){resp->deleteLater();});
}
void Client::send_block(const qblocks::Block& block_)
{
    auto node_block_=new Node_block(block_);
    connect(node_block_,&Node_block::finished,this,[=](){
        auto blockid_=Client::post_api_core_v2_blocks(node_block_->block_.get_Json());
        node_block_->deleteLater();
        connect(blockid_,&Node_blockID::finished,this,[=](){
            emit last_blockid(blockid_->id);
            blockid_->deleteLater();
        });
    });
    auto info_=get_api_core_v2_info();
    connect(info_,&Node_info::finished,this,[=](){
        node_block_->set_pv(info_->protocol_version);
        auto tips_=get_api_core_v2_tips();
        connect(tips_,&Node_tips::finished,this,[=](){
            node_block_->set_parents(tips_->tips);
            if(info_->min_pow_score&&!(info_->pow_feature))
            {
                auto nfinder_=new qpow::nonceFinder();

                connect(nfinder_,&qpow::nonceFinder::nonce_found,this,[=](const quint64 &nonce)
                {
                    node_block_->set_nonce(nonce);
                    nfinder_->deleteLater();
                });
                connect(nfinder_,&qpow::nonceFinder::nonce_not_found,this,[=](){
                    nfinder_->deleteLater();
                    node_block_->deleteLater();                    
                    send_block(block_);
                });
                nfinder_->set_Min_Pow_Score(info_->min_pow_score);
                nfinder_->calculate(node_block_->ready());
            }
            else
            {
                node_block_->set_nonce(0);
            }

            tips_->deleteLater();
            info_->deleteLater();
        });
    });


}
Response* Client::get_api_core_v2_blocks_blockId_metadata(const QString& blockId)
{
    return get_reply_rest("/api/core/v2/blocks/"+blockId+"/metadata");
}
Response* Client::get_api_core_v2_outputs_outputId(const QString& outputId)
{
    return get_reply_rest("/api/core/v2/outputs/"+outputId);
}
Response* Client::get_api_core_v2_outputs_outputId_metadata(const QString& outputId)
{
    return get_reply_rest("/api/core/v2/outputs/"+outputId+"/metadata");
}

}
