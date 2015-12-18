/*
 * Copyright (C) Kedixa Liu
 *  kedixa@outlook.com
 * ID3 算法基础实现
 *
 */

#include "ID3.h"

const double ID3::log2 = log(2);
ID3::ID3()
{
	root = nullptr;
}

/*
 * function: ID3::del_tree 删除决策树
 *
 */
bool ID3::_del_tree(ID3_Node* p)
{
	if(p == nullptr)
		return true;
	for(int i = 0; i < (int)p->child.size(); ++i)
	{
		_del_tree(p->child[i]);
		delete p->child[i];
	}
	return true;
}

/*
 * function: ID3::clear 清空数据
 *
 */
bool ID3::clear()
{
	headers.clear();
	datas.clear();
	attr_to_int.clear();
	int_to_attr.clear();
	bool f = _del_tree(root);
	if(f) root = nullptr;
	return f;
}

/*
 * function: ID3::set_data 设置数据集
 * d: 数据集
 * s: 目标属性
 * h: 各个属性的值
 * 
 */
bool ID3::set_data(vvs& d, std::string& s, vs& h)
{
	bool f = clear();
	if(!f) return f;
	target_attr = s;
	headers = h;

	// 分配空间
	num_attr = (int)headers.size();
	num_data = (int)d.size();
	attr_to_int.resize(num_attr);
	datas.resize(num_attr);
	int_to_attr.resize(num_attr);

	// 数据集映射到整数域，便于计算
	for(int i = 0; i < num_data; ++i)
	{
		auto& e = d[i];
		for(int j = 0; j < num_attr; ++j)
		{
			auto it = attr_to_int[j].find(e[j]);
			if(it == attr_to_int[j].end())
			{
				attr_to_int[j][e[j]] = (int)int_to_attr[j].size();
				it = attr_to_int[j].find(e[j]);
				int_to_attr[j].push_back(e[j]);
			}
			datas[j].push_back(it->second);
		}
	}
	
	// 确定目标属性
	target = -1;
	for(int i = 0; i < num_attr; ++i)
		if(headers[i] == target_attr)
		{
			target = i;
			break;
		}
	if(target == -1)
		return false;

	return true;
}

/*
 * function: ID3::_entropy 计算数据集的熵
 * data_list: 数据集
 *
 */
double ID3::_entropy(const vi& data_list)
{
	double entropy_sv = 0;
	double data_size = (double)data_list.size();
	vi tmp;
	tmp.resize(int_to_attr[target].size(), 0);
	for(auto &j : data_list)
		++tmp[datas[target][j]];
	for(int i = 0; i < (int)tmp.size(); ++i)
	{
		if(tmp[i] != 0)
		{
			double d = tmp[i] / data_size;
			entropy_sv -= d * log(d) / log2;
		}
	}
	return entropy_sv;
}

/*
 * function: ID3::_gain 获取以当前属性进行分支对数据集的增益
 * data_list: 数据集
 * attr: 属性
 *
 */
double ID3::_gain(const vi& data_list, int attr)
{
	double entropy_S = _entropy(data_list);
	double data_size = (double)data_list.size();
	// 按照当前属性将数据集分成子数据集
	vvi data_i;
	data_i.resize(int_to_attr[attr].size());
	for(auto& j : data_list)
		data_i[datas[attr][j]].push_back(j);
	// 求信息增益
	double sub_entropy = 0;
	for(int i = 0; i < (int)data_i.size(); ++i)
		sub_entropy += data_i[i].size() * _entropy(data_i[i]) / data_size;

	return entropy_S - sub_entropy;
}

/*
 *
 * function: _find_best_attr 查找当前最优的属性
 * data_list: 当前数据集
 * attr_list: 当前属性集
 * current_gain: 保存最优属性对应的增益值
 *
 */
int ID3::_find_best_attr(const vi& data_list, 
		const vi& attr_list,
		double& best_gain)
{
	best_gain = -1;
	int best_attr = -1;
	for(auto &attr : attr_list)
	{
		double current_gain = _gain(data_list, attr);
		if(best_gain < current_gain)
			best_gain = current_gain, best_attr = attr;
	}
	assert(best_attr != -1);
	return best_attr;
}

/*
 * function: ID3::build_tree 递归构建决策树
 * data_list: 当前数据集列表
 * attr_list: 当前属性列表
 *
 */

ID3_Node* ID3::_build_tree(const vi& data_list, const vi& attr_list)
{
	// 当前数据集中的一个目标属性值
	auto &dl = data_list;
	auto &al = attr_list;
	int one_of_target = datas[target][dl[0]];
	ID3_Node* node = new ID3_Node;
	bool flag = std::all_of(dl.begin(), dl.end(),[&](int x)
			{
			return (datas[target][x] == one_of_target);
			});
	// 如果当前数据集所有目标属性相同，则建立一个叶子结点
	if(flag)
	{
		node->attr_index = target;
		node->target_value = one_of_target;
		node->gain = 0;
	}
	// 如果当前属性值为空，则建立一个叶结点
	// 值为当前数据集中出现次数最多的目标属性值
	else if(al.empty())
	{
		node->attr_index = target;
		vi tmp;
		tmp.resize(int_to_attr[target].size(), 0);
		for(auto& i : dl)
			++tmp[datas[target][i]];
		int max_index = -1, max_value = -1;
		for(int i = 0; i < (int)tmp.size(); ++i)
			if(max_value < tmp[i])
				max_value = tmp[i], max_index = i;
		node->target_value = max_index;
		node->gain = 0;
	}
	else
	{
		// 使用最好的属性来建立分支
		double current_gain;
		int best_attr = _find_best_attr(dl, al, current_gain);
		node->attr_index = best_attr;
		node->gain = current_gain;
		int attr_size = (int)int_to_attr[best_attr].size();

		// 按照属性值分离数据集
		vvi data_i;
		data_i.resize(attr_size);
		for(auto& x : dl)
			data_i[datas[best_attr][x]].push_back(x);

		for(int i = 0; i < attr_size; ++i)
		{
			//FIXME attr_value 多余？
			node->attr_value.push_back(i);
			ID3_Node* p = nullptr;

			// 如果当前属性值对应的数据集为空，建立叶结点
			// 值为原数据集中出现最多的目标属性的值
			if(data_i[i].empty())
			{
				vi tmp;
				tmp.resize(attr_size, 0);
				for(auto& x : dl)
					++tmp[datas[target][x]];
				int max_index = -1, max_value = -1;
				for(int k = 0; k < attr_size; ++k)
					if(max_value < tmp[k])
						max_value = tmp[k], max_index = k;

				p = new ID3_Node;
				p->attr_index = target;
				p->target_value = max_index;
				p->gain = 0;
			}
			else
			{
				// 分支结点的属性列表
				vi attr_i;
				for(auto& j : al)
					if(j != best_attr) attr_i.push_back(j);
				p = _build_tree(data_i[i], attr_i);
			}

			node->child.push_back(p);
		}
	}

	return node;
}

/*
 * function: ID3::run 开始构建决策树
 *
 */
bool ID3::run()
{
	// 初始化属性列表和数据列表
	vi attr_list;
	vi data_list;
	attr_list.resize(num_attr - 1);
	data_list.resize(num_data);
	for(int i = 0; i < num_data; ++i)
		data_list[i] = i;
	for(int i = 0; i < num_attr - 1; ++i)
		attr_list[i] = i >= target ? i + 1 : i;

	// 递归构建决策树
	root = _build_tree(data_list, attr_list);
	if(root == nullptr)
		return false;
	return true;
}

/*
 * function: _print
 * p: 子树根节点指针
 * depth: 当前深度
 * out: 输出流对象
 *
 */
int ID3::_print(ID3_Node* p, int depth, std::ostream& out)
{
	if(p == nullptr)
		return 0;
	out<<std::string(depth, '.');
	if(p->attr_index == target)
	{
		out<<target_attr<<' '<<
			int_to_attr[target][p->target_value]<<std::endl;
		return 0;
	}
	out<<headers[p->attr_index]<<"\tgain:\t"<<p->gain<<std::endl;
	for(int i = 0; i < (int)p->child.size(); ++i)
		_print(p->child[i], depth + 1, out);
	return 0;
}

int ID3::_print_dot(ID3_Node* p, int& node_index, std::ostream& out)
{
	if(p == nullptr)
		return 0;
	int current_index = node_index ++;
	if(p->attr_index == target)
	{
		out<<"\tnode"<<current_index<<" [shape = none, label = \""
			<<int_to_attr[target][p->target_value]<<"\"];\n";
		return current_index;
	}
	out<<"\tnode"<<current_index<<" [shape = box, label = \""
		<<headers[p->attr_index]<<"\"];\n";
	for(int i = 0; i < (int)p->child.size(); ++i)
	{
		int this_child_index = _print_dot(p->child[i], node_index, out);
		out<<"\tnode"<<current_index<<" -> node"<<this_child_index<<
			" [label = \""<<int_to_attr[p->attr_index][i]<<"\"];\n";
	}
	return current_index;
}
/*
 * function: print 输出树形结构
 * out: 输出流对象
 *
 */
void ID3::print(std::ostream& out = std::cout)
{
	if(root == nullptr)
		return;
	_print(root, 0, out);
}

/*
 * fucntion: ID3::print_dot 以dot格式输出
 *
 */
void ID3::print_dot(std::ostream& out = std::cout)
{
	if(root == nullptr)
		return;
	out<<"digraph G\n{\n";
	int node_index = 0;
	_print_dot(root, node_index,  out);
	out<<"}\n";
	out.flush();
}

ID3::~ID3()
{
	clear();
}
