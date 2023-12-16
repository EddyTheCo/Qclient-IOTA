#pragma once
#include"block/qblock.hpp"
#include"client/qnode_response.hpp"
#include"client/qnode_info.hpp"
#include"client/qnode_tips.hpp"
#include"client/qnode_blockId.hpp"
#include"client/qnode_block.hpp"
#include"client/qnode_outputs.hpp"
#include <QNetworkAccessManager>
#include <QString>
#include<QHash>

namespace qiota{

	class QCLIENT_EXPORT Client: public QObject
	{

		Q_OBJECT
		public:
			Client(QObject *parent = nullptr);
			enum ClientState {
				Disconnected = 0,
				Connected
			};
			void send_block(const qblocks::Block& block_);
            void getFundsFromFaucet(const QString& bech32Address,
                                           const QUrl & faucetAddress=QUrl("https://faucet.testnet.shimmer.network"));
			template<qblocks::Output::types outtype>
                Node_outputs* get_outputs(const QString& filter)
				{
					auto outputids=get_api_indexer_v1_outputs<outtype>(filter);
                    auto node_outs_=new Node_outputs(this);
                    connect(outputids,&Response::returned,node_outs_,[=,this](QJsonValue data ){
							auto transid=data["items"].toArray();
							node_outs_->size_+=transid.size();
							outputids->deleteLater();
							if(transid.size()==0)node_outs_->fill();
							for(auto v:transid)
							{
							auto output=get_api_core_v2_outputs_outputId(v.toString());
							QObject::connect(output,&Response::returned,node_outs_,[=](QJsonValue data){
									node_outs_->fill(data);
									output->deleteLater();
									});
							}

							});
                    return node_outs_;

				}
			static quint64 get_deposit(const std::shared_ptr<const qblocks::Output>& out,const Node_info *info)
			{
				return out->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);
			}

            void setNodeAddress(const QUrl &nodeAddress);
            QUrl getNodeAddress(void)const{return m_nodeAddress;}
            QString JWT;
			Node_info* get_api_core_v2_info(void);
            ClientState state(void)const{return m_state;}



signals:
			void last_blockid(qblocks::c_array id);
            void stateChanged();

		private:
            void setState(ClientState state){if(m_state!=state){m_state=state;emit stateChanged();}}
			Response*  get_reply_rest(const QString& path, const QString &query=QString());
            Response*  post_reply_rest(const QString& path, const QJsonObject& payload );


			Node_tips* get_api_core_v2_tips(void);
            Node_blockID* post_api_core_v2_blocks(const QJsonObject& block_);
			Node_block* get_api_core_v2_blocks_blockId(const QString& blockId);
			Response* get_api_core_v2_blocks_blockId_metadata(const QString& blockId);

			Response* get_api_core_v2_outputs_outputId(const QString& outputId);
			Response* get_api_core_v2_outputs_outputId_metadata(const QString& outputId);
			template<qblocks::Output::types outtype>
				Response* get_api_indexer_v1_outputs(const QString& filter)
				{
                    return get_reply_rest("/api/indexer/v1/outputs"+qblocks::Output::typesstr[outtype],filter);
				}

            QUrl m_nodeAddress;
			QNetworkAccessManager* nam;
            ClientState m_state;
	};

}
