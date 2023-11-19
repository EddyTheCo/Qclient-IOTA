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

			template<qblocks::Output::types outtype>
				void get_outputs(Node_outputs* node_outs_,const QString& filter)
				{
					auto outputids=get_api_indexer_v1_outputs<outtype>(filter);
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

				}
			static quint64 get_deposit(const std::shared_ptr<const qblocks::Output>& out,const Node_info *info)
			{
				return out->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);
			}
			void set_node_address(const QUrl node_address_m);
			QUrl get_node_address(void)const{return rest_node_address_;}
			QString get_jwt(void)const{return JWT;}
			void set_jwt(const QString jwt_m){JWT=jwt_m;}
			Node_info* get_api_core_v2_info(void);
			ClientState state(void)const{return state_;}
			QJsonObject info()const{return info_;}


signals:
			void last_blockid(qblocks::c_array id);
			void stateChanged();

		private:
			void set_State(ClientState state_m){if(state_m!=state_){state_=state_m;emit stateChanged();}}
			Response*  get_reply_rest(const QString& path, const QString &query=QString());
			Response*  post_reply_rest(const QString& path, const QJsonObject& payload )const;


			Node_tips* get_api_core_v2_tips(void);
			Node_blockID* post_api_core_v2_blocks(const QJsonObject& block_)const;
			Node_block* get_api_core_v2_blocks_blockId(const QString& blockId);
			Response* get_api_core_v2_blocks_blockId_metadata(const QString& blockId);

			Response* get_api_core_v2_outputs_outputId(const QString& outputId);
			Response* get_api_core_v2_outputs_outputId_metadata(const QString& outputId);
			template<qblocks::Output::types outtype>
				Response* get_api_indexer_v1_outputs(const QString& filter)
				{
					return get_reply_rest("/api/indexer/v1/outputs/"+qblocks::Output::typesstr[outtype],filter);
				}


			QUrl rest_node_address_;
			QNetworkAccessManager* nam;
			QString JWT;
			ClientState state_;
		        QJsonObject info_;
	};

}
