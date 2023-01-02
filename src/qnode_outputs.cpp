#include"client/qnode_outputs.hpp"
#include<QJsonObject>
#include<QDebug>
namespace qiota{

Node_outputs::Node_outputs(void):size_(0)
{

}
std::vector<std::shared_ptr<qblocks::Input>> Node_outputs::create_inputs(std::vector<Node_output> outs_,const quint64 amount_need_it,
                                                                  qblocks::c_array& Inputs_Commitments, quint64& amount)
{
    std::vector<std::shared_ptr<qblocks::Input>> inputs;
    while(((amount_need_it)?amount<amount_need_it:1)&&!outs_.empty())
    {
        const auto v=outs_.back();
        if(!v.metadata().is_spent_)
        {
            inputs.push_back(std::shared_ptr<qblocks::Input>(new qblocks::UTXO_Input(v.metadata().transaction_id_,
                                                                                     v.metadata().output_index_)));
            qblocks::c_array prevOutputSer;
            prevOutputSer.from_object<qblocks::Output>(*(v.output()));
            auto Inputs_Commitment1=QCryptographicHash::hash(prevOutputSer, QCryptographicHash::Blake2b_256);
            Inputs_Commitments.append(Inputs_Commitment1);
            amount+=std::dynamic_pointer_cast<qblocks::Basic_Output>(v.output())->amount();
        }
        outs_.pop_back();
    }
    return inputs;
}

Node_output::Node_output(QJsonValue data):metadata_(data["metadata"]),out_(qblocks::Output::from_<const QJsonValue>(data["output"]))
{

}
void Node_outputs::fill(QJsonValue data)
{
   outs_.push_back(Node_output(data));
   qDebug()<<"fill";

    if(outs_.size()==size_)
    {
        qDebug()<<"fill finished";
        emit finished();
    }

}
}
