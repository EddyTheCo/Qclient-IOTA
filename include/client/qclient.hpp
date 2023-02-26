#pragma once
/**
 *  https://github.com/iotaledger/tips/blob/main/tips/TIP-0025/tip-0025.md
 *
 **/
#include"block/qblock.hpp"
#include"client/qnode_response.hpp"
#include"client/qnode_info.hpp"
#include"client/qnode_tips.hpp"
#include"client/qnode_blockId.hpp"
#include"client/qnode_block.hpp"
#include"client/qnode_outputs.hpp"
#include <QNetworkAccessManager>
#include <QString>

namespace qiota{

class Client: public QObject
{

    Q_OBJECT
public:
    Client(QObject *parent = nullptr);
    enum ClientState {
        Disconnected = 0,
        Connected
    };
    void send_block(const qblocks::Block& block_)const;
    void get_basic_outputs(Node_outputs* node_outs_,const QString& filter)const;
    void get_nft_outputs(Node_outputs* node_outs_,const QString& filter)const;
    void set_node_address(const QUrl node_address_m);
    QUrl get_node_address(void)const{return rest_node_address_;}
    QString get_jwt(void)const{return JWT;}
    void set_jwt(const QString jwt_m){JWT=jwt_m;}
    Node_info* get_api_core_v2_info(void)const;
    ClientState state(void)const{return state_;}

signals:
    void last_blockid(qblocks::c_array id)const;
    void stateChanged(Client::ClientState state);

private:
    void set_State(ClientState state_m){if(state_m!=state_){state_=state_m;emit stateChanged(state_);}}
    Response*  get_reply_rest(const QString& path, const QString &query="")const;
    Response*  post_reply_rest(const QString& path, const QJsonObject& payload )const;


    Node_tips* get_api_core_v2_tips(void)const;
    Node_blockID* post_api_core_v2_blocks(const QJsonObject& block_)const;
    Node_block* get_api_core_v2_blocks_blockId(const QString& blockId)const;
    Response* get_api_core_v2_blocks_blockId_metadata(const QString& blockId)const;

    Response* get_api_core_v2_outputs_outputId(const QString& outputId)const;
    Response* get_api_core_v2_outputs_outputId_metadata(const QString& outputId)const;
    Response* get_api_indexer_v1_outputs_basic(const QString& filter)const;
    Response* get_api_indexer_v1_outputs_nft(const QString& filter)const;
    Response* get_api_indexer_v1_outputs_nft_nftId(const QString& nftId)const;

    QUrl rest_node_address_;
    QNetworkAccessManager* nam;
    QString JWT;
    ClientState state_;
};

};
