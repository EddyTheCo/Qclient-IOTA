#pragma once

#include"block/qblock.hpp"
#include"client/qnode_response.hpp"
#include <QJsonValue>
namespace qiota{

	class Node_output
	{
		public:
			Node_output(QJsonValue data);
			std::shared_ptr<qblocks::Output> output(void)const{return out_;}
			qblocks::Output_Metadata_Response metadata(void)const{return metadata_;}
			QJsonObject get_Json()const
			{
				QJsonObject var;
				var.insert("metadata",metadata_.get_Json());
				var.insert("output",out_->get_Json());

				switch(out_->type()) {
					case qblocks::Output::NFT_typ:
						var.insert("chainAddress",qblocks::NFT_Address(metadata_.outputid_.hash<QCryptographicHash::Blake2b_256>()).get_Json()) ;
						break;
					case qblocks::Output::Alias_typ:
						var.insert("chainAddress",qblocks::Alias_Address(metadata_.outputid_.hash<QCryptographicHash::Blake2b_256>()).get_Json()) ;
						break;
					case qblocks::Output::Foundry_typ:
						var.insert("FoundryId",out_->get_id().toHexString()) ;
						break;
					case qblocks::Output::Basic_typ:
						break;
				}
				return var;
			};
		private:
			std::shared_ptr<qblocks::Output> out_;
			qblocks::Output_Metadata_Response metadata_;
	};

	class QCLIENT_EXPORT Node_outputs : public QObject
	{
		Q_OBJECT
		public:
			Node_outputs(QObject *parent = nullptr);
			std::vector<Node_output> outs_;
			size_t size_;

			public slots:
				void fill(QJsonValue data);
			void fill(){emit finished();};
		signals:
			void finished();


	};

};
