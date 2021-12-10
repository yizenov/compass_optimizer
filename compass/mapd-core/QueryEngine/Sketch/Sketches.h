#ifndef QUERYENGINE_SKETCHES_H
#define QUERYENGINE_SKETHCES_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

#include "SketchUtil.h"
#include "XIS.h"

class FAGMS_Sketch {
 private:
  unsigned int buckets_no;
  unsigned int rows_no;
  unsigned int keys_no;
  Xi_CW2B** xi_bucket;
  Xi_EH3** xi_pm1;
  double* sketch_elem;
  double** separate_sketch_elem;

 public:
  FAGMS_Sketch() : buckets_no(0), rows_no(0), keys_no(0), xi_bucket(NULL), xi_pm1(NULL), sketch_elem(NULL), separate_sketch_elem(NULL) {}

  FAGMS_Sketch(unsigned int _buckets_no, unsigned int _rows_no, unsigned int _keys_no) : buckets_no(_buckets_no), rows_no(_rows_no), keys_no(_keys_no) {
    xi_bucket = new Xi_CW2B*[_keys_no];
    xi_pm1 = new Xi_EH3*[_keys_no];
    sketch_elem = new double[buckets_no * rows_no];
    for (unsigned i = 0; i < buckets_no * rows_no; i++)
      sketch_elem[i] = 0.0;
    separate_sketch_elem = new double*[_keys_no];
  }

  FAGMS_Sketch(unsigned int _buckets_no, unsigned int _rows_no, Xi_CW2B* _xi_bucket, Xi_EH3* _xi_pm1)
      : buckets_no(_buckets_no), rows_no(_rows_no), keys_no(1) {
    xi_bucket = new Xi_CW2B*[1];
    xi_bucket[0] = _xi_bucket;
    xi_pm1 = new Xi_EH3*[1];
    xi_pm1[0] = _xi_pm1;

    sketch_elem = new double[buckets_no * rows_no];
    for (unsigned i = 0; i < buckets_no * rows_no; i++)
      sketch_elem[i] = 0.0;
    separate_sketch_elem = new double*[1];
  }

  FAGMS_Sketch(unsigned int _buckets_no,
               unsigned int _rows_no,
               unsigned int _keys_no,
               Xi_CW2B** _xi_bucket,
               Xi_EH3** _xi_pm1)
      : buckets_no(_buckets_no), rows_no(_rows_no), keys_no(_keys_no), xi_bucket(_xi_bucket), xi_pm1(_xi_pm1) {
    sketch_elem = new double[buckets_no * rows_no];
    for (unsigned i = 0; i < buckets_no * rows_no; i++)
      sketch_elem[i] = 0.0;
    separate_sketch_elem = new double*[_keys_no];
  }

  void Copy_Sketch_Template(FAGMS_Sketch* sketch_template, unsigned int column_index, bool seeds_only) {
    xi_bucket[column_index] = sketch_template->xi_bucket[0];
    xi_pm1[column_index] = sketch_template->xi_pm1[0];
    if (!seeds_only) {
      separate_sketch_elem[column_index] = sketch_template->sketch_elem;
    } else {
      separate_sketch_elem[column_index] = new double[buckets_no * rows_no];
      for (unsigned i = 0; i < buckets_no * rows_no; i++)
        separate_sketch_elem[column_index][i] = 0.0;
    }
  }

  void Copy_Sketch_Seeds(std::pair<Xi_CW2B*, Xi_EH3*>* sketch_seeds, unsigned int column_index) {
    xi_bucket[column_index] = sketch_seeds->first;
    xi_pm1[column_index] =sketch_seeds->second;
    separate_sketch_elem[column_index] = new double[buckets_no * rows_no];
    for (unsigned i = 0; i < buckets_no * rows_no; i++)
      separate_sketch_elem[column_index][i] = 0.0;
  }

  // Cloning is needed to avoid updating template data
  void Clone_Sketch(FAGMS_Sketch* original_sketch, double** template_sketch_elements, std::vector<unsigned>& column_indices) {
    buckets_no = original_sketch->buckets_no;
    rows_no = original_sketch->rows_no;
    keys_no = original_sketch->keys_no;
    xi_bucket = original_sketch->xi_bucket;
    xi_pm1 = original_sketch->xi_pm1;
    sketch_elem = new double[buckets_no * rows_no];
    separate_sketch_elem = new double*[keys_no];
    for (unsigned i = 0; i < keys_no; i++) {
      separate_sketch_elem[i] = new double[buckets_no * rows_no];
      unsigned col_idx = column_indices[i];
      for (unsigned j = 0; j < buckets_no * rows_no; j++)
        separate_sketch_elem[i][j] = template_sketch_elements[col_idx][j];
    }
  }

  void Reset_Sketch(FAGMS_Sketch* original_sketch) {
    buckets_no = original_sketch->buckets_no;
    rows_no = original_sketch->rows_no;
    keys_no = original_sketch->keys_no;
    xi_bucket = original_sketch->xi_bucket;
    xi_pm1 = original_sketch->xi_pm1;
    sketch_elem = new double[buckets_no * rows_no];
    for (unsigned i = 0; i < buckets_no * rows_no; i++)
      sketch_elem[i] = 0.0;
    separate_sketch_elem = new double*[keys_no];
  }

  virtual ~FAGMS_Sketch() {
    for(unsigned i = 0; i < keys_no; i++) {
      //delete[] xi_bucket[i];  // cannot delete the seeds since they are shared
      //delete[] xi_pm1[i];
      delete[] separate_sketch_elem[i];
    }
    delete[] sketch_elem;
  }

  void Clear_Sketch() {
    for (unsigned i = 0; i < buckets_no * rows_no; i++)
      sketch_elem[i] = 0.0;
  }

  void Set_Sketch_With_Max() {
    for (unsigned i = 0; i < buckets_no * rows_no; i++)
      sketch_elem[i] = std::numeric_limits<unsigned>::max();
  }

  void Merge_Zero() {
    for (unsigned i = 0; i < buckets_no * rows_no; i++)
      if (sketch_elem[i] == std::numeric_limits<unsigned>::max())
        sketch_elem[i] = 0;
  }

  void Choose_Sketch_Min_Val(int col_idx) {
    for (unsigned i = 0; i < buckets_no * rows_no; i++) {
      if ((separate_sketch_elem[col_idx][i] != 0) && (std::abs(sketch_elem[i]) > std::abs(separate_sketch_elem[col_idx][i])))
        sketch_elem[i] = separate_sketch_elem[col_idx][i];
    }
  }

  void Set_Sketch_Sparse(int col_idx) {
    for (unsigned i = 0; i < buckets_no * rows_no; i++) {
      if (separate_sketch_elem[col_idx][i] != 0)
        sketch_elem[i] = separate_sketch_elem[col_idx][i];
    }
  }

  void Max_Non_Zero_Buckets(int col_idx, unsigned & out_max) {
    unsigned max_c = 0;
    for (unsigned i = 0; i < rows_no; i++) {
      unsigned cnt = 0;
      for (unsigned j = 0; j < buckets_no; j++)
        if (separate_sketch_elem[col_idx][i*buckets_no+j] != 0) cnt += 1;
      if (cnt > max_c) max_c = cnt;
    }
    out_max = max_c;
  }

  void Add_Sketch_Values(int col_idx) {
    for (unsigned i = 0; i < buckets_no * rows_no; i++)
      sketch_elem[i] = separate_sketch_elem[col_idx][i];
  }

  double* get_sketch_elements() { return sketch_elem; }
  void set_sketch_elements(double* elements) { sketch_elem = elements; }
  double** get_separate_sketch_elements() { return separate_sketch_elem; }
  void set_separate_sketch_elements(double** separate_elements) { separate_sketch_elem = separate_elements; }

  unsigned int get_buckets_no() { return buckets_no; }
  unsigned int get_rows_no() { return rows_no; }
  unsigned int get_keys_no() { return keys_no; }
  Xi_CW2B** get_seed1_vals() { return xi_bucket; }
  Xi_EH3** get_seed2_vals() { return xi_pm1; }

  void Update_Sketch(unsigned int _key, double _func) {
    for (unsigned i = 0; i < rows_no; i++) {
      int bucket = (int)xi_bucket[0][i].element(_key);
      sketch_elem[i * buckets_no + bucket] += (xi_pm1[0][i].element(_key) * _func);
    }
  }

  void Update_Sketch(unsigned int* _key, double _func) {
    for (unsigned i = 0; i < rows_no; i++) {
      for (unsigned j = 0; j < keys_no; j++) {
        int b = (int)(xi_bucket[j][i].element(_key[j]));
        sketch_elem[i * buckets_no + b] += (xi_pm1[j][i].element(_key[j]) * _func);
      }
    }
  }

  void Merge_Sketch(FAGMS_Sketch& _me) {
    for (unsigned i = 0; i < rows_no * buckets_no; i++)
      sketch_elem[i] += _me.sketch_elem[i];
  }

  void Output_Values() {
    std::cout << "sketch elements values:";
    for (unsigned i = 0; i < rows_no; i++) {
      for (unsigned j = 0; j < buckets_no; j++)
        std::cout << " " << sketch_elem[i * buckets_no + j];
    }
    std::cout << std::endl;
  }

  // TODO: post pre-process -> convert all 0s to 1s : OR start the counters with 1s
  double Size_Of_Join(FAGMS_Sketch* _s1) {
    double* basic_est = new double[rows_no];
    for (unsigned i = 0; i < rows_no; i++) {
      basic_est[i] = 0.0;
      for (unsigned j = 0; j < buckets_no; j++) {
        double t1 = sketch_elem[i * buckets_no + j];
        if (t1 == 0)
          t1 = pow(10, -4); // 1; to avoid zero values
        double t2 = _s1->sketch_elem[i * buckets_no + j];
        if (t2 == 0)
          t2 = pow(10, -4); // 1; // to avoid zero values
        basic_est[i] = basic_est[i] + t1*t2;// fabs(t1 * t2); // to avoid negative values
        //basic_est[i] = basic_est[i] / (keys_no * _s1->get_keys_no()); // to avoid overestimation
      }
      basic_est[i] = fabs(basic_est[i]);
    }

    double result = Median(basic_est, rows_no);

    delete[] basic_est;

    return result;
  }

  double Size_Of_Join(FAGMS_Sketch** _s1, unsigned int _skNo) {
    double* basic_est = new double[rows_no];
    for (unsigned i = 0; i < rows_no; i++) {
      basic_est[i] = 0.0;
      for (unsigned j = 0; j < buckets_no; j++) {
        double bp = sketch_elem[i * buckets_no + j];
        //double denominator = keys_no;
        if (bp == 0)
          bp = pow(10, -4); // 1; to avoid zero values
        for (unsigned k = 0; k < _skNo; k++) {
          double t = _s1[k]->sketch_elem[i * buckets_no + j];
          if (t == 0)
            t = pow(10, -4); // 1; to avoid zero values
          bp = bp * t; //fabs(bp * t); // to avoid negative values
          //denominator = denominator * _s1[k]->get_keys_no();
        }
        basic_est[i] = basic_est[i] + bp;
        //basic_est[i] = basic_est[i] / denominator; // to avoid overestimation
      }
      basic_est[i] = fabs(basic_est[i]);
    }

    double result = Median(basic_est, rows_no);

    delete[] basic_est;

    return result;
  }

  double Self_Join_Size() {
    double* basic_est = new double[rows_no];
    for (unsigned i = 0; i < rows_no; i++) {
      basic_est[i] = 0.0;
      for (unsigned j = 0; j < buckets_no; j++)
        basic_est[i] = basic_est[i] + sketch_elem[i * buckets_no + j] * sketch_elem[i * buckets_no + j];
    }

    double result = Median(basic_est, rows_no);

    delete[] basic_est;

    return result;
  }

  void PrintSketch() {
    for (unsigned j = 0; j < keys_no; j++) {
      std::cout << "xi_bucket:: ";
      for (unsigned i = 0; i < rows_no; i++) { std::cout << " (" << xi_bucket[j][i].seeds[0] << "--" << xi_bucket[j][i].seeds[1] << ") "; }
      std::cout << std::endl << "xi_pm1:: ";
      for (unsigned i = 0; i < rows_no; i++) { std::cout << " (" << xi_pm1[j][i].seeds[0] << "--" << xi_pm1[j][i].seeds[1] << ") "; }
      std::cout << std::endl << "sep. sketch values:: " << std::endl;
      for (unsigned i = 0; i < rows_no; i++) {
        std::cout << "\trow_id: " << i + 1 << " ::";
        for (unsigned k = i * buckets_no; k < (i + 1) * buckets_no; k++) { std::cout << " " << separate_sketch_elem[j][k]; }
        std::cout << std::endl;
      }
      std::cout << std::endl;
    }
    std::cout << std::endl <<std::flush;
  }

  // overload print operator
  friend std::ostream& operator<<(std::ostream& _os, FAGMS_Sketch& _sk);
};

inline std::ostream& operator<<(std::ostream& _os, FAGMS_Sketch& _sk) {
  _os << "{R : " << _sk.rows_no << " B : " << _sk.buckets_no << "}" << std::endl;
  for (unsigned i = 0; i < 1; /*_sk.rows_no;*/ i++) {
    _os << "\t(Xi : " << _sk.xi_bucket[i] << " V : {";
    for (unsigned j = 0; j < _sk.buckets_no / 39; j++)
      _os << " " << _sk.sketch_elem[i * _sk.buckets_no + j];
    _os << "})";
  }
  _os << "]";
  return _os;
}

#endif  // QUERYENGINE_SKETHCES_H
