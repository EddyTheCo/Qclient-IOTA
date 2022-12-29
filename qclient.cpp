#include"client/qclient.hpp"
#include"pow/qpow.hpp"
#include<QJsonDocument>
#include<QJsonObject>
#include<iostream>
namespace qiota{

Client::Client(const QUrl& rest_node_address):
    rest_node_address_(rest_node_address),nam(new QNetworkAccessManager())
{
nam->connectToHost(rest_node_address_.toEncoded(),14265);

};
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
Node_info* Client::get_api_core_v2_info(void)const
{
    return new Node_info(get_reply_rest("/api/core/v2/info"));
}
Node_tips* Client::get_api_core_v2_tips()const
{
    return new Node_tips(get_reply_rest("/api/core/v2/tips"));
}
Node_blockID *Client::post_api_core_v2_blocks(const QJsonObject& block_)const
{
    return new Node_blockID(post_reply_rest("/api/core/v2/blocks",block_));
}
Node_block* Client::get_api_core_v2_blocks_blockId(const QString& blockId)const
{
    return new Node_block(get_reply_rest("/api/core/v2/blocks/"+blockId));
}

void Client::send_block(const qblocks::Block& block_)const
{
    auto node_block_=new Node_block(block_);
    auto info_=get_api_core_v2_info();
    auto tips_=get_api_core_v2_tips();
    auto nfinder_=new qpow::nonceFinder();
    QObject::connect(info_,&Node_info::finished,node_block_,[=](){node_block_->set_pv(info_->protocol_version);});
    QObject::connect(info_,&Node_info::finished,nfinder_,[=](){nfinder_->set_Min_Pow_Score(info_->min_pow_score);
        info_->deleteLater();
    });
    QObject::connect(tips_,&Node_tips::finished,node_block_,[=](){node_block_->set_parents(tips_->tips);
        tips_->deleteLater();
    });
    QObject::connect(node_block_,&Node_block::ready,nfinder_,&qpow::nonceFinder::calculate);
    QObject::connect(nfinder_,&qpow::nonceFinder::nonce_found,node_block_,&Node_block::set_nonce);
    QObject::connect(nfinder_,&qpow::nonceFinder::nonce_found,nfinder_,&QObject::deleteLater);
    QObject::connect(nfinder_,&qpow::nonceFinder::nonce_not_found,this,[=](){
        node_block_->deleteLater();
        nfinder_->deleteLater();
        send_block(block_);
    });
    QObject::connect(node_block_,&Node_block::finished,this,[=](){

        auto blockid_=Client::post_api_core_v2_blocks(node_block_->block_.get_Json());
        node_block_->deleteLater();
        QObject::connect(blockid_,&Node_blockID::finished,this,[=](){
            emit last_blockid(blockid_->id);
            blockid_->deleteLater();
        });
    });


}
void Client::get_basic_outputs(Node_outputs* node_outs_,const QString& filter)const
{
    auto outputids=get_api_indexer_v1_outputs_basic(filter);
    QObject::connect(outputids,&Response::returned,node_outs_,[=](QJsonValue data ){
        auto transid=data["items"].toArray();
        node_outs_->size_+=transid.size();
        outputids->deleteLater();
        for(auto v:transid)
        {
            auto output=get_api_core_v2_outputs_outputId(v.toString());
            QObject::connect(output,&Response::returned,node_outs_,[=](QJsonValue data){
                node_outs_->fill(data);
                output->deleteLater();
            });
        }

    });

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
