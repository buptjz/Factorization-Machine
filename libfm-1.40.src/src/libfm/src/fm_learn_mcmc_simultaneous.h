/*
 MCMC and ALS based learning for factorization machines
 - this file contains the learning procedure including evaluations
 
 Based on the publication(s):
 * Steffen Rendle (2010): Factorization Machines, in Proceedings of the 10th IEEE International Conference on Data Mining (ICDM 2010), Sydney, Australia.
 * Steffen Rendle, Zeno Gantner, Christoph Freudenthaler, Lars Schmidt-Thieme (2011): Fast Context-aware Recommendations with Factorization Machines, in Proceedings of the 34th international ACM SIGIR conference on Research and development in information retrieval (SIGIR 2011), Beijing, China.
 * Christoph Freudenthaler, Lars Schmidt-Thieme, Steffen Rendle (2011): Bayesian Factorization Machines, in NIPS Workshop on Sparse Representation and Low-rank Approximation (NIPS-WS 2011), Spain.
 * Steffen Rendle (2012): Factorization Machines with libFM, ACM Transactions on Intelligent Systems and Technology (TIST 2012).
 * Steffen Rendle (2013): Scaling Factorization Machines to Relational Data, in Proceedings of the 39th international conference on Very Large Data Bases (VLDB 2013), Trento, Italy.
 
 Author:   Steffen Rendle, http://www.libfm.org/
 modified: 2013-07-04
 
 Copyright 2010-2013 Steffen Rendle, see license.txt for more information
 */

#ifndef FM_LEARN_MCMC_SIMULTANEOUS_H_
#define FM_LEARN_MCMC_SIMULTANEOUS_H_

#include "fm_learn_mcmc.h"


class fm_learn_mcmc_simultaneous : public fm_learn_mcmc {
protected:
    
    virtual void _learn(Data& train, Data& test) {
        
        uint num_complete_iter = 0;//请问有什么用？
        
        // make a collection of datasets that are predicted jointly
        int num_data = 2;
        DVector<Data*> main_data(num_data);
        DVector<e_q_term*> main_cache(num_data);//存储每一个instance的 e 和 q
        main_data(0) = &train;
        main_data(1) = &test;
        main_cache(0) = cache;
        main_cache(1) = cache_test;
        
        
        predict_data_and_write_to_eterms(main_data, main_cache);//预测y，算法的第4行
        if (task == TASK_REGRESSION) {
            // remove the target from each prediction, because: e(c) := \hat{y}(c) - target(c)
            for (uint c = 0; c < train.num_cases; c++) {
                cache[c].e = cache[c].e - train.target(c);//每一个instance都有一个e，并且ei = yi - yi_predict，见公式23
            }
            
        } else if (task == TASK_CLASSIFICATION) {
            // for Classification: remove from e not the target but a sampled value from a truncated normal
            // for initializing, they are not sampled but initialized with meaningful values:
            // -1 for the negative class and +1 for the positive class (actually these are the values that are already in the target and thus, we can do the same as for regression; but note that other initialization strategies would need other techniques here:
            for (uint c = 0; c < train.num_cases; c++) {
                cache[c].e = cache[c].e - train.target(c);
            }
            
        } else {
            throw "unknown task";
        }
        
        //开始迭代
        for (uint i = num_complete_iter; i < num_iter; i++) {
            double iteration_time = getusertime();
            clock_t iteration_time3 = clock();
            double iteration_time4 = getusertime4();
            
            //以下一对uint的值，不知道干啥用的目前，其实是统计这些参数是否是空值
            nan_cntr_w0 = 0;
            inf_cntr_w0 = 0;
            nan_cntr_w = 0;
            inf_cntr_w = 0;
            nan_cntr_v = 0;
            inf_cntr_v = 0;
            nan_cntr_alpha = 0;
            inf_cntr_alpha = 0;
            nan_cntr_w_mu = 0;
            inf_cntr_w_mu = 0;
            nan_cntr_w_lambda = 0;
            inf_cntr_w_lambda = 0;
            nan_cntr_v_mu = 0;
            inf_cntr_v_mu = 0;
            nan_cntr_v_lambda = 0;
            inf_cntr_v_lambda = 0;
            
            //采样，算法第6~24行
            draw_all(train);
			
            
            if ((nan_cntr_alpha > 0) || (inf_cntr_alpha > 0)) {
                std::cout << "#nans in alpha:\t" << nan_cntr_alpha << "\t#inf_in_alpha:\t" << inf_cntr_alpha << std::endl;
            }
            if ((nan_cntr_w0 > 0) || (inf_cntr_w0 > 0)) {
                std::cout << "#nans in w0:\t" << nan_cntr_w0 << "\t#inf_in_w0:\t" << inf_cntr_w0 << std::endl;
            }
            if ((nan_cntr_w > 0) || (inf_cntr_w > 0)) {
                std::cout << "#nans in w:\t" << nan_cntr_w << "\t#inf_in_w:\t" << inf_cntr_w << std::endl;
            }
            if ((nan_cntr_v > 0) || (inf_cntr_v > 0)) {
                std::cout << "#nans in v:\t" << nan_cntr_v << "\t#inf_in_v:\t" << inf_cntr_v << std::endl;
            }
            if ((nan_cntr_w_mu > 0) || (inf_cntr_w_mu > 0)) {
                std::cout << "#nans in w_mu:\t" << nan_cntr_w_mu << "\t#inf_in_w_mu:\t" << inf_cntr_w_mu << std::endl;
            }
            if ((nan_cntr_w_lambda > 0) || (inf_cntr_w_lambda > 0)) {
                std::cout << "#nans in w_lambda:\t" << nan_cntr_w_lambda << "\t#inf_in_w_lambda:\t" << inf_cntr_w_lambda << std::endl;
            }
            if ((nan_cntr_v_mu > 0) || (inf_cntr_v_mu > 0)) {
                std::cout << "#nans in v_mu:\t" << nan_cntr_v_mu << "\t#inf_in_v_mu:\t" << inf_cntr_v_mu << std::endl;
            }
            if ((nan_cntr_v_lambda > 0) || (inf_cntr_v_lambda > 0)) {
                std::cout << "#nans in v_lambda:\t" << nan_cntr_v_lambda << "\t#inf_in_v_lambda:\t" << inf_cntr_v_lambda << std::endl;
            }
            
            
            // predict test and train
            predict_data_and_write_to_eterms(main_data, main_cache);
            // (prediction of train is not necessary but it increases numerical stability)
            double acc_train = 0.0;
            double rmse_train = 0.0;
            if (task == TASK_REGRESSION) {
                // evaluate test and store it
                for (uint c = 0; c < test.num_cases; c++) {
                    double p = cache_test[c].e;
                    pred_this(c) = p;
                    p = std::min(max_target, p);
                    p = std::max(min_target, p);
                    pred_sum_all(c) += p;
                    if (i >= 5) {
                        pred_sum_all_but5(c) += p;
                    }
                }
                
                // Evaluate the training dataset and update the e-terms
                for (uint c = 0; c < train.num_cases; c++) {
                    double p = cache[c].e;
                    p = std::min(max_target, p);
                    p = std::max(min_target, p);
                    double err = p - train.target(c);
                    rmse_train += err*err;
                    cache[c].e = cache[c].e - train.target(c);
                }
                rmse_train = std::sqrt(rmse_train/train.num_cases);
                
            } else if (task == TASK_CLASSIFICATION) {
                // evaluate test and store it
                for (uint c = 0; c < test.num_cases; c++) {
                    double p = cache_test[c].e;
                    p = cdf_gaussian(p);
                    pred_this(c) = p;
                    pred_sum_all(c) += p;
                    if (i >= 5) {
                        pred_sum_all_but5(c) += p;
                    }
                }
                
                // Evaluate the training dataset and update the e-terms
                uint _acc_train = 0;
                for (uint c = 0; c < train.num_cases; c++) {
                    double p = cache[c].e;
                    p = cdf_gaussian(p);
                    if (((p >= 0.5) && (train.target(c) > 0.0)) || ((p < 0.5) && (train.target(c) < 0.0))) {
                        _acc_train++;
                    }
                    
                    double sampled_target;
                    if (train.target(c) >= 0.0) {
                        if (do_sample) {
                            sampled_target = ran_left_tgaussian(0.0, cache[c].e, 1.0);
                        } else {
                            // the target is the expected value of the truncated normal
                            double mu = cache[c].e;
                            double phi_minus_mu = exp(-mu*mu/2.0) / sqrt(3.141*2);
                            double Phi_minus_mu = cdf_gaussian(-mu);
                            sampled_target = mu + phi_minus_mu / (1-Phi_minus_mu);
                        }
                    } else {
                        if (do_sample) {
                            sampled_target = ran_right_tgaussian(0.0, cache[c].e, 1.0);
                        } else {
                            // the target is the expected value of the truncated normal
                            double mu = cache[c].e;
                            double phi_minus_mu = exp(-mu*mu/2.0) / sqrt(3.141*2);
                            double Phi_minus_mu = cdf_gaussian(-mu);
                            sampled_target = mu - phi_minus_mu / Phi_minus_mu;
                        }
                    }
                    cache[c].e = cache[c].e - sampled_target;
                }
                acc_train = (double) _acc_train / train.num_cases;
                
            } else {
                throw "unknown task";
            }
            
            iteration_time = (getusertime() - iteration_time);
            iteration_time3 = clock() - iteration_time3;
            iteration_time4 = (getusertime4() - iteration_time4);
            if (log != NULL) {
                log->log("time_learn", iteration_time);
                log->log("time_learn2", (double)iteration_time3 / CLOCKS_PER_SEC);
                log->log("time_learn4", (double)iteration_time4);
            }
            
            
            // Evaluate the test data sets
            if (task == TASK_REGRESSION) {
                double rmse_test_this, mae_test_this, rmse_test_all, mae_test_all, rmse_test_all_but5, mae_test_all_but5;
                _evaluate(pred_this, test.target, 1.0, rmse_test_this, mae_test_this, num_eval_cases);
                _evaluate(pred_sum_all, test.target, 1.0/(i+1), rmse_test_all, mae_test_all, num_eval_cases);
                _evaluate(pred_sum_all_but5, test.target, 1.0/(i-5+1), rmse_test_all_but5, mae_test_all_but5, num_eval_cases);
                
                std::cout << "#Iter=" << std::setw(3) << i << "\tTrain=" << rmse_train << "\tTest=" << rmse_test_all << std::endl;
                
                if (log != NULL) {
                    log->log("rmse", rmse_test_all);
                    log->log("mae", mae_test_all);
                    log->log("rmse_mcmc_this", rmse_test_this);
                    log->log("rmse_mcmc_all", rmse_test_all);
                    log->log("rmse_mcmc_all_but5", rmse_test_all_but5);
                    
                    if (num_eval_cases < test.target.dim) {
                        double rmse_test2_this, mae_test2_this, rmse_test2_all, mae_test2_all;//, rmse_test2_all_but5, mae_test2_all_but5;
                        _evaluate(pred_this, test.target, 1.0, rmse_test2_this, mae_test2_this, num_eval_cases, test.target.dim);
                        _evaluate(pred_sum_all, test.target, 1.0/(i+1), rmse_test2_all, mae_test2_all, num_eval_cases, test.target.dim);
                        //log->log("rmse_mcmc_test2_this", rmse_test2_this);
                        //log->log("rmse_mcmc_test2_all", rmse_test2_all);
                    }
                    log->newLine();
                }
            } else if (task == TASK_CLASSIFICATION) {
                double acc_test_this, acc_test_all, acc_test_all_but5,
                ll_test_this, ll_test_all, ll_test_all_but5;
                _evaluate_class(pred_this, test.target, 1.0, acc_test_this, ll_test_this, num_eval_cases);
                _evaluate_class(pred_sum_all, test.target, 1.0/(i+1), acc_test_all, ll_test_all, num_eval_cases);
                _evaluate_class(pred_sum_all_but5, test.target, 1.0/(i-5+1), acc_test_all_but5, ll_test_all_but5, num_eval_cases);
                
                std::cout << "#Iter=" << std::setw(3) << i << "\tTrain=" << acc_train << "\tTest=" << acc_test_all << "\tTest(ll)=" << ll_test_all << std::endl;
                
                if (log != NULL) {
                    log->log("accuracy", acc_test_all);
                    log->log("acc_mcmc_this", acc_test_this);
                    log->log("acc_mcmc_all", acc_test_all);
                    log->log("acc_mcmc_all_but5", acc_test_all_but5);
                    log->log("ll_mcmc_this", ll_test_this);
                    log->log("ll_mcmc_all", ll_test_all);
                    log->log("ll_mcmc_all_but5", ll_test_all_but5);
                    
                    if (num_eval_cases < test.target.dim) {
                        double acc_test2_this, acc_test2_all,
                        ll_test2_this, ll_test2_all;
                        _evaluate_class(pred_this, test.target, 1.0, acc_test2_this, ll_test2_this, num_eval_cases, test.target.dim);
                        _evaluate_class(pred_sum_all, test.target, 1.0/(i+1), acc_test2_all, ll_test2_all, num_eval_cases, test.target.dim);
                        //log->log("acc_mcmc_test2_this", acc_test2_this);
                        //log->log("acc_mcmc_test2_all", acc_test2_all);
                    }
                    log->newLine();
                }
                
            } else {
                throw "unknown task";
            }
        }
    }
    
    void _evaluate(DVector<double>& pred, DVector<DATA_FLOAT>& target, double normalizer, double& rmse, double& mae, uint from_case, uint to_case) {
        assert(pred.dim == target.dim);
        double _rmse = 0;
        double _mae = 0;
        uint num_cases = 0;
        for (uint c = std::max((uint) 0, from_case); c < std::min((uint)pred.dim, to_case); c++) {
            double p = pred(c) * normalizer;
            p = std::min(max_target, p);
            p = std::max(min_target, p);
            double err = p - target(c);
            _rmse += err*err;
            _mae += std::abs((double)err);
            num_cases++;
        }
        
        rmse = std::sqrt(_rmse/num_cases);
        mae = _mae/num_cases;
        
    }
    
    void _evaluate_class(DVector<double>& pred, DVector<DATA_FLOAT>& target, double normalizer, double& accuracy, double& loglikelihood, uint from_case, uint to_case) {
        double _loglikelihood = 0.0;
        uint _accuracy = 0;
        uint num_cases = 0;
        for (uint c = std::max((uint) 0, from_case); c < std::min((uint)pred.dim, to_case); c++) {
            double p = pred(c) * normalizer;
            if (((p >= 0.5) && (target(c) > 0.0)) || ((p < 0.5) && (target(c) < 0.0))) {
                _accuracy++;
            }
            double m = (target(c)+1.0)*0.5;
            double pll = p;
            if (pll > 0.99) { pll = 0.99; }
            if (pll < 0.01) { pll = 0.01; }
            _loglikelihood -= m*log10(pll) + (1-m)*log10(1-pll);
            num_cases++;
        }
        loglikelihood = _loglikelihood/num_cases;
        accuracy = (double) _accuracy / num_cases;
    }
    
    
    void _evaluate(DVector<double>& pred, DVector<DATA_FLOAT>& target, double normalizer, double& rmse, double& mae, uint& num_eval_cases) {
        _evaluate(pred, target, normalizer, rmse, mae, 0, num_eval_cases);
    }
    
    void _evaluate_class(DVector<double>& pred, DVector<DATA_FLOAT>& target, double normalizer, double& accuracy, double& loglikelihood, uint& num_eval_cases) {
        _evaluate_class(pred, target, normalizer, accuracy, loglikelihood, 0, num_eval_cases);
    }
};

#endif /*FM_LEARN_MCMC_SIMULTANEOUS_H_*/
