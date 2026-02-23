#include "ComputerVision.h"



NAMESPACE_UPP


void EigenVV(const FloatMat& A, FloatMat* vects, FloatMat* vals) {
	ASSERT(vects || vals);
	int n = A.cols;
	int dt = 1;
	
	static thread_local Vector<double> a_db;
	static thread_local Vector<double> w_db;
	static thread_local Vector<double> v_db;
	
	a_db.SetCount(n * n);
	w_db.SetCount(n);
	if (vects) v_db.SetCount(n * n);
	
	for (int i = 0; i < n * n; i++) a_db[i] = A.data[i];
	
		JacobiImpl(a_db, n, w_db, (vects ? &v_db : NULL), n, n);
	
		/*
		Cout() << "EigenVV results:\n";
		for (int i = 0; i < n; i++) {
			Cout() << "  val[" << i << "] = " << w_db[i] << " vec = [";
			if (vects) {
				for (int j = 0; j < n; j++) Cout() << v_db[j * n + i] << (j == n - 1 ? "" : ", ");
			}
			Cout() << "]\n";
		}
		*/
	
		if (vects) {
	
		vects->SetSize(n, n, dt);
		for (int i = 0; i < n * n; i++) vects->data[i] = (float)v_db[i];
	}
	
	if (vals) {
		vals->SetSize(1, n, dt);
		for (int i = 0; i < n; i++) vals->data[i] = (float)w_db[i];
	}
}


END_UPP_NAMESPACE
