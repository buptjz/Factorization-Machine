/*
 Dense Matrix and Vectors
 
 Author:   Steffen Rendle, http://www.libfm.org/
 modified: 2012-06-08
 
 Copyright 2010-2012 Steffen Rendle, see license.txt for more information
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <vector>
#include <assert.h>
#include <iostream>
#include <fstream>
#include "../util/memory.h"
#include "../util/random.h"

const uint DVECTOR_EXPECTED_FILE_ID = 1;
const uint DMATRIX_EXPECTED_FILE_ID = 1001;

struct dmatrix_file_header {
	uint id;
	uint type_size;
	uint num_rows;
	uint num_cols;
};

template <typename T> class DMatrix {//矩阵，和vector类似，只不过是2D的
public:
    T** value;
    
    std::vector<std::string> col_names;
    uint dim1, dim2;
    
    T get(uint x, uint y) {
        //assert((x < dim1) && (y < dim2));
        return value[x][y];
    }
    
    DMatrix(uint p_dim1, uint p_dim2) {
        dim1 = 0;
        dim2 = 0;
        value = NULL;
        setSize(p_dim1, p_dim2);
    }
    
    DMatrix() {
        dim1 = 0;
        dim2 = 0;
        value = NULL;
    }
    
    ~DMatrix() {
        if (value != NULL) {
            MemoryLog::getInstance().logFree("dmatrix", sizeof(T*), dim1);
            delete [] value[0];
            MemoryLog::getInstance().logFree("dmatrix", sizeof(T), dim1*dim2);
            delete [] value;
        }
    }
    
    void assign(DMatrix<T>& v) {
        if ((v.dim1 != dim1) || (v.dim2 != dim2)) { setSize(v.dim1, v.dim2); }
        for (uint i = 0; i < dim1; i++) {
            for (uint j = 0; j < dim2; j++) {
                value[i][j] = v.value[i][j];
            }
        }
    }
    void init(T v) {
        for (uint i = 0; i < dim1; i++) {
            for (uint i2 = 0; i2 < dim2; i2++) {
                value[i][i2] = v;
            }
        }
    }
    void setSize(uint p_dim1, uint p_dim2) {
        if ((p_dim1 == dim1) && (p_dim2 == dim2)) {
            return;
        }
        if (value != NULL) {
            MemoryLog::getInstance().logFree("dmatrix", sizeof(T*), dim1);
            delete [] value[0];
            MemoryLog::getInstance().logFree("dmatrix", sizeof(T), dim1*dim2);
            delete [] value;
        }
        dim1 = p_dim1;
        dim2 = p_dim2;
        MemoryLog::getInstance().logNew("dmatrix", sizeof(T*), dim1);
        value = new T*[dim1];
        MemoryLog::getInstance().logNew("dmatrix", sizeof(T), dim1*dim2);
        value[0] = new T[dim1 * dim2];
        for (unsigned i = 1; i < dim1; i++) {
            value[i] = value[0] + i * dim2;
        }
        col_names.resize(dim2);
        for (unsigned i = 1; i < dim2; i++) {
            col_names[i] = "";
        }
    }
    
    T& operator() (unsigned x, unsigned y) {
   		//	assert((x < dim1) && (y < dim2));
        return value[x][y];
    }
    T operator() (unsigned x, unsigned y) const {
   		//	assert((x < dim1) && (y < dim2));
        return value[x][y];
    }
    
    T* operator() (unsigned x) const {
   		//	assert((x < dim1));
        return value[x];
    }
    
    void save(std::string filename, bool has_header = false) {
        std::ofstream out_file (filename.c_str());
        if (out_file.is_open())	{
            if (has_header) {
                for (uint i_2 = 0; i_2 < dim2; i_2++) {
                    if (i_2 > 0) {
                        out_file << "\t";
                    }
                    out_file << col_names[i_2];
                }
                out_file << std::endl;
            }
            for (uint i_1 = 0; i_1 < dim1; i_1++) {
                for (uint i_2 = 0; i_2 < dim2; i_2++) {
                    if (i_2 > 0) {
                        out_file << "\t";
                    }
                    out_file << value[i_1][i_2];
                }
                out_file << std::endl;
            }
            out_file.close();
        } else {
            std::cout << "Unable to open file " << filename;
        }
    }
    
    void saveToBinaryFile(std::string filename) {
        std::cout << "writing to " << filename << std::endl; std::cout.flush();
        std::ofstream out(filename.c_str(), std::ios_base::out | std::ios_base::binary);
        if (out.is_open()) {
            dmatrix_file_header fh;
            fh.id = DMATRIX_EXPECTED_FILE_ID;
            fh.num_rows = dim1;
            fh.num_cols = dim2;
            fh.type_size = sizeof(T);
            out.write(reinterpret_cast<char*>(&fh), sizeof(fh));
            for (uint i = 0; i < dim1; i++) {
                out.write(reinterpret_cast<char*>(value[i]), sizeof(T)*dim2);
            }
            out.close();
        } else {
            throw "could not open " + filename;
        }
    }
    
    void loadFromBinaryFile(std::string filename) {
        std::cout << "reading " << filename << std::endl; std::cout.flush();
        std::ifstream in(filename.c_str(), std::ios_base::in | std::ios_base::binary);
        if (in.is_open()) {
            dmatrix_file_header fh;
            in.read(reinterpret_cast<char*>(&fh), sizeof(fh));
            assert(fh.id == DMATRIX_EXPECTED_FILE_ID);
            assert(fh.type_size == sizeof(T));
            setSize(fh.num_rows, fh.num_cols);
            for (uint i = 0; i < dim1; i++) {
                in.read(reinterpret_cast<char*>(value[i]), sizeof(T)*dim2);
            }
            in.close();
        } else {
            throw "could not open " + filename;
        }
    }
    
    void load(std::string filename) {
        std::ifstream in_file (filename.c_str());
        if (! in_file.is_open()) {
            throw "Unable to open file " + filename;
        }
        for (uint i_1 = 0; i_1 < dim1; i_1++) {
            for (uint i_2 = 0; i_2 < dim2; i_2++) {
                T v;
                in_file >> v;
                value[i_1][i_2] = v;
            }
        }
        in_file.close();
    }
    
};

template <typename T> class DVector {//Vector
public:
    uint dim;//维度
    T* value;//值
    DVector() {//构造函数：维度和值
        dim = 0;
        value = NULL;
    }
    DVector(uint p_dim) {//用维度构造vector
        dim = 0;
        value = NULL;
        setSize(p_dim);
    }
    ~DVector() {
        if (value != NULL) {
            MemoryLog::getInstance().logFree("dvector", sizeof(T), dim);
            delete [] value;
        }
    }
    T get(uint x) {//获取vector的第x个元素
        return value[x];
    }
    void setSize(uint p_dim) {//设置数组维度
        if (p_dim == dim) { return; }
        if (value != NULL) {
            MemoryLog::getInstance().logFree("dvector", sizeof(T), dim);
            delete [] value;
        }
        dim = p_dim;
        MemoryLog::getInstance().logNew("dvector", sizeof(T), dim);
        value = new T[dim];
    }
    T& operator() (unsigned x) {//符号重载
        return value[x];
    }
    T operator() (unsigned x) const {//重载
        return value[x];
    }
    void init(T v) {//用一个数值初始化全部vector元素
        for (uint i = 0; i < dim; i++) {
            value[i] = v;
        }
    }
    void assign(T* v) {//赋值操作，用指针赋值
        if (v->dim != dim) { setSize(v->dim); }
        for (uint i = 0; i < dim; i++) {
            value[i] = v[i];
        }
    }
    void assign(DVector<T>& v) {//用引用赋值
        if (v.dim != dim) { setSize(v.dim); }
        for (uint i = 0; i < dim; i++) {
            value[i] = v.value[i];
        }
    }
    void save(std::string filename) {//将vector保存到一个文件中
        std::ofstream out_file (filename.c_str());
        if (out_file.is_open())	{
            for (uint i = 0; i < dim; i++) {
                out_file << value[i] << std::endl;
            }
            out_file.close();
        } else {
            std::cout << "Unable to open file " << filename;
        }
    }
    
    void saveToBinaryFile(std::string filename) {//保存到二进制文件中
        std::ofstream out (filename.c_str(), std::ios_base::out | std::ios_base::binary);
        if (out.is_open())	{
            uint file_version = DVECTOR_EXPECTED_FILE_ID;
            uint data_size = sizeof(T);
            uint num_rows = dim;
            out.write(reinterpret_cast<char*>(&file_version), sizeof(file_version));
            out.write(reinterpret_cast<char*>(&data_size), sizeof(data_size));
            out.write(reinterpret_cast<char*>(&num_rows), sizeof(num_rows));
            out.write(reinterpret_cast<char*>(value), sizeof(T)*dim);
            out.close();
        } else {
            std::cout << "Unable to open file " << filename;
        }
    }
    
    
    void load(std::string filename) {//读取文件，生成vector
        std::ifstream in_file (filename.c_str());
        if (! in_file.is_open()) {
            throw "Unable to open file " + filename;
        }
        for (uint i = 0; i < dim; i++) {
            T v;
            in_file >> v;
            value[i] = v;
        }
        in_file.close();
    }
    
    
    void loadFromBinaryFile(std::string filename) {
        std::ifstream in (filename.c_str(), std::ios_base::in | std::ios_base::binary);
        if (in.is_open())	{
            uint file_version;
            uint data_size;
            uint num_rows;
            in.read(reinterpret_cast<char*>(&file_version), sizeof(file_version));
            in.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
            in.read(reinterpret_cast<char*>(&num_rows), sizeof(num_rows));
            assert(file_version == DVECTOR_EXPECTED_FILE_ID);
            assert(data_size == sizeof(T));
            setSize(num_rows);
            in.read(reinterpret_cast<char*>(value), sizeof(T)*dim);
            in.close();
        } else {
            std::cout << "Unable to open file " << filename;
        }   			
    }
};


class DVectorDouble : public DVector<double> {
public:
    void init_normal(double mean, double stdev) {//用x~N(μ,σ2)初始化
        for (uint i_2 = 0; i_2 < dim; i_2++) {
            value[i_2] = ran_gaussian(mean, stdev);
        }
    }
};

class DMatrixDouble : public DMatrix<double> {
public:
    void init(double mean, double stdev) {	
        for (uint i_1 = 0; i_1 < dim1; i_1++) {
            for (uint i_2 = 0; i_2 < dim2; i_2++) {
                value[i_1][i_2] = ran_gaussian(mean, stdev);
            }
        }
    }
    void init_column(double mean, double stdev, int column) {//初始化某一列
        for (uint i_1 = 0; i_1 < dim1; i_1++) {
            value[i_1][column] = ran_gaussian(mean, stdev);
        }
    }
};


#endif /*MATRIX_H_*/
