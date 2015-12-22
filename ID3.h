/*
 * Copyright (C) Kedixa Liu
 *  kedixa@outlook.com
 * ID3 算法基础实现
 *
 */

#include<iostream>
#include<algorithm>
#include<vector>
#include<string>
#include<unordered_map>
#include<cassert>

#ifndef ID3_H_
#define ID3_H_

namespace kedixa{

class ID3;
class ID3_Node
{
	int    attr_index; // 属性下标
	int    target_value; // 如果该结点为叶结点，则保存属性的值
	double gain; // 信息增益

	std::vector<ID3_Node*> child; // 子结点的指针
	friend class ID3;
};

class ID3
{
	typedef std::vector<std::string>            vs;
	typedef std::vector<vs>                     vvs;
	typedef std::vector<int>                    vi;
	typedef std::vector<vi>                     vvi;
	typedef std::unordered_map<std::string,int> usi;
	typedef std::vector<usi>                    vusi;
private:
	std::string target_attr;  // 目标属性
	vs          headers;      // 各个属性的名称
	vvi         datas;        // 将属性映射到整数后的数据集，并按列保存
	vusi        attr_to_int;  // 属性到整数的映射
	vvs         int_to_attr;  // 整数到属性的映射
	vi          attrs_size;   // 每个属性有多少个不同的属性值
	int         num_attr;     // 属性数量
	int         num_data;     // 数据集大小
	int         target;       // 目标属性在headers中的下标
	ID3_Node*   root;         // 决策树根结点指针
	const static double log2;

	bool      init(); // 初始化
	ID3_Node* _build_tree(const vi&, const vi&);
	int       _find_best_attr(const vi&, const vi&, double&);
	int       _print(ID3_Node*, int, std::ostream&);
	int       _print_dot(ID3_Node*, int&, std::ostream&);
	bool      _del_tree(ID3_Node*);
	double    _gain(const vi&, int);
	double    _entropy(const vi&);

public:
	ID3();
	bool set_data(vvs&, std::string&, vs&);
	bool run();
	void print(std::ostream&);
	void print_dot(std::ostream&);
	bool clear();
	~ID3();
};

} // namespace kedixa
#endif // ID3_H_
