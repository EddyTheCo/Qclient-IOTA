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
#include <QNetworkAccessManager>
#include <QString>

namespace qiota{

class Client: public QObject
{

      Q_OBJECT
public:
    Client(const QUrl& rest_node_address=QUrl(""));


    void send_block(const qblocks::Block& block_)const;

signals:
void last_blockid(qblocks::c_array id)const;

private:
    Response*  get_reply_rest(const QString& path, const QString &query="")const;
    Response*  post_reply_rest(const QString& path, const QJsonObject& payload )const;

    Node_info* get_api_core_v2_info(void)const;
    Node_tips* get_api_core_v2_tips(void)const;
    Node_blockID* post_api_core_v2_blocks(const QJsonObject& block_)const;
    Node_block* get_api_core_v2_blocks_blockId(const QString& blockId)const;
    Response* get_api_core_v2_blocks_blockId_metadata(const QString& blockId)const;

    Response* get_api_core_v2_outputs_outputId(const QString& outputId)const;
    Response* get_api_core_v2_outputs_outputId_metadata(const QString& outputId)const;
    Response* get_api_indexer_v1_outputs_basic(const QString& filter)const;


    Node_info* info;
    QUrl rest_node_address_;
    QNetworkAccessManager* nam;
};

};
