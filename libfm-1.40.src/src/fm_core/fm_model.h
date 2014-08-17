/*
	Model for Factorization Machines

	Based on the publication(s):
	Steffen Rendle (2010): Factorization Machines, in Proceedings of the 10th IEEE International Conference on Data Mining (ICDM 2010), Sydney, Australia.

	Author:   Steffen Rendle, http://www.libfm.org/
	modified: 2012-01-04

	Copyright 2010-2012 Steffen Rendle, see license.txt for more information
*/

#ifndef FM_MODEL_H_
#define FM_MODEL_H_

#include "../util/matrix.h"
#include "../util/fmatrix.h"

#include "fm_data.h"


class fm_model {
	private:
		DVector<double> m_sum, m_sum_sqr; //k维（因子维度）
	public:
		double w0;          //w0
		DVectorDouble w;    //w1 ~ wp
		DMatrixDouble v;    //v<1,1> ~ v<p,k>

	public:
		// the following values should be set:
		uint num_attribute;//含有的 feature 数量
		
		bool k0, k1;//是否是使用w0和 是否使用w1~wp
		int num_factor;//因子的维度
		
		double reg0;//图2(b)中的 λ0 的初值
		double regw, regv;//图2(b)中的 λw 和 λv 的初值
		
		double init_stdev;//初始化的stdev，默认是0.1
		double init_mean;
		
		fm_model();
		void debug();
		void init();
		double predict(sparse_row<FM_FLOAT>& x);
		double predict(sparse_row<FM_FLOAT>& x, DVector<double> &sum, DVector<double> &sum_sqr);
	
};



fm_model::fm_model() {
	num_factor = 0;
	init_mean = 0;
	init_stdev = 0.01;
	reg0 = 0.0;
	regw = 0.0;
	regv = 0.0; 
	k0 = true;
	k1 = true;
}

void fm_model::debug() {
	std::cout << "num_attributes=" << num_attribute << std::endl;
	std::cout << "use w0=" << k0 << std::endl;
	std::cout << "use w1=" << k1 << std::endl;
	std::cout << "dim v =" << num_factor << std::endl;
	std::cout << "reg_w0=" << reg0 << std::endl;
	std::cout << "reg_w=" << regw << std::endl;
	std::cout << "reg_v=" << regv << std::endl; 
	std::cout << "init ~ N(" << init_mean << "," << init_stdev << ")" << std::endl;
}

void fm_model::init() {
	w0 = 0;
	w.setSize(num_attribute);
	v.setSize(num_factor, num_attribute);//v<p,k>
	w.init(0);//w1~wp = 0 所有w都是0，包括上面的w0
	v.init(init_mean, init_stdev);//所有v都通过~N(μ，σ2)初始化 即ran_gaussian(mean, stdev);
	m_sum.setSize(num_factor);
	m_sum_sqr.setSize(num_factor);
}

double fm_model::predict(sparse_row<FM_FLOAT>& x) {
	return predict(x, m_sum, m_sum_sqr);		
}

double fm_model::predict(sparse_row<FM_FLOAT>& x, DVector<double> &sum, DVector<double> &sum_sqr) {
	double result = 0;
	if (k0) {	
		result += w0;
	}
	if (k1) {
		for (uint i = 0; i < x.size; i++) {
			assert(x.data[i].id < num_attribute);
			result += w(x.data[i].id) * x.data[i].value;//w[i] * x[i] and i from 1 to p
		}
	}
	//use formula 5
	for (int f = 0; f < num_factor; f++) {//f = 1 to k
		sum(f) = 0;
		sum_sqr(f) = 0;
		for (uint i = 0; i < x.size; i++) {//j = 1 to p
			double d = v(f,x.data[i].id) * x.data[i].value;
			sum(f) += d;
			sum_sqr(f) += d*d;
		}
		result += 0.5 * (sum(f)*sum(f) - sum_sqr(f));
	}
	return result;
}

#endif /*FM_MODEL_H_*/
