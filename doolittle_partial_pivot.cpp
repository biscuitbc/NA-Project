#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Matrix = std::vector<std::vector<double>>;

// compact[i][j] stores U(i,j) on/above the diagonal and L(i,j) below it.
// L has an implicit unit diagonal. permutation[i] is the original row now
// occupying row i, so (P*A)(i,j) = A(permutation[i],j).
struct LUFactorization {
    Matrix compact;
    std::vector<std::size_t> permutation;
};

// 对方阵执行带部分选主元的 Doolittle 分解，原地将工作矩阵改写为紧凑存储 C。
// 返回的置换向量 pi 满足 (P*A)(i,j) = A(pi[i],j)；遇到零主元或非法输入时抛出异常。
LUFactorization doolittlePartialPivot(Matrix a) {
    const std::size_t n = a.size();
    if (n == 0) {
        throw std::invalid_argument("矩阵不能为空");
    }

    for (std::size_t i = 0; i < n; ++i) {
        if (a[i].size() != n) {
            throw std::invalid_argument("系数矩阵必须是方阵");
        }
        for (double value : a[i]) {
            if (!std::isfinite(value)) {
                throw std::invalid_argument("矩阵元素必须是有限数");
            }
        }
    }

    std::vector<std::size_t> permutation(n);
    for (std::size_t i = 0; i < n; ++i) {
        permutation[i] = i;
    }

    // In-place Gaussian elimination with partial pivoting. The multipliers
    // are kept below the diagonal, producing the compact Doolittle storage.
    for (std::size_t k = 0; k < n; ++k) {
        std::size_t pivotRow = k;
        double largest = std::abs(a[k][k]);
        for (std::size_t i = k + 1; i < n; ++i) {
            const double candidate = std::abs(a[i][k]);
            if (candidate > largest) {  // ties keep the first (smallest) row
                largest = candidate;
                pivotRow = i;
            }
        }

        if (largest == 0.0) {
            throw std::runtime_error(
                "矩阵奇异：当前列剩余候选主元全为零（第 " +
                std::to_string(k + 1) + " 列）");
        }

        if (pivotRow != k) {
            std::swap(a[k], a[pivotRow]);
            std::swap(permutation[k], permutation[pivotRow]);
        }

        for (std::size_t i = k + 1; i < n; ++i) {
            a[i][k] /= a[k][k];  // save L(i,k) in the position being eliminated
            if (!std::isfinite(a[i][k])) {
                throw std::overflow_error("计算乘子时发生浮点溢出");
            }
            const double multiplier = a[i][k];
            for (std::size_t j = k + 1; j < n; ++j) {
                a[i][j] -= multiplier * a[k][j];
                if (!std::isfinite(a[i][j])) {
                    throw std::overflow_error("消元过程中发生浮点溢出");
                }
            }
        }
    }

    return {std::move(a), std::move(permutation)};
}

// Solve A*x=b from P*A=L*U using the compact factors.
// 使用紧凑 LU 分解求解 A*x=b：先解 L*y=P*b，再解 U*x=y。
// 输入 b 按原矩阵行序排列，函数通过分解结果中的置换向量应用 P。
std::vector<double> solve(const LUFactorization& lu,
                          const std::vector<double>& b) {
    const std::size_t n = lu.compact.size();
    if (b.size() != n || lu.permutation.size() != n) {
        throw std::invalid_argument("右端向量长度与矩阵阶数不一致");
    }

    std::vector<double> y(n, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        if (!std::isfinite(b[lu.permutation[i]])) {
            throw std::invalid_argument("右端向量元素必须是有限数");
        }
        double value = b[lu.permutation[i]];  // apply P to b
        for (std::size_t j = 0; j < i; ++j) {
            value -= lu.compact[i][j] * y[j];
        }
        y[i] = value;  // diagonal of L is implicitly 1
    }

    std::vector<double> x(n, 0.0);
    for (std::size_t reverse = n; reverse > 0; --reverse) {
        const std::size_t i = reverse - 1;
        double value = y[i];
        for (std::size_t j = i + 1; j < n; ++j) {
            value -= lu.compact[i][j] * x[j];
        }
        const double pivot = lu.compact[i][i];
        if (pivot == 0.0) {
            throw std::runtime_error("回代时遇到零主元");
        }
        x[i] = value / pivot;
        if (!std::isfinite(x[i])) {
            throw std::overflow_error("回代过程中发生浮点溢出");
        }
    }
    return x;
}

// 按紧凑存储约定打印矩阵 C：下三角为 L 的乘子，对角线及上三角为 U。
void printCompact(const Matrix& c) {
    std::cout << "紧凑存储 C（下三角为 L 的乘子，对角线及上三角为 U）：\n";
    for (const auto& row : c) {
        for (double value : row) {
            std::cout << std::setw(14) << value;
        }
        std::cout << '\n';
    }
}

// 从紧凑矩阵 C 恢复并打印 L、U；L 的单位对角线按定义补为 1。
void printFactors(const Matrix& c) {
    const std::size_t n = c.size();
    std::cout << "L =\n";
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            std::cout << std::setw(14) << (i == j ? 1.0 : (i > j ? c[i][j] : 0.0));
        }
        std::cout << '\n';
    }

    std::cout << "U =\n";
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            std::cout << std::setw(14) << (i <= j ? c[i][j] : 0.0);
        }
        std::cout << '\n';
    }
}

// 命令行入口：读取矩阵并执行分解，可选读取右端向量并求解线性方程组。
int main() {
    // Input format:
    // n
    // A's n rows (n numbers per row)
    // optional: 1 followed by n entries of b to also solve A*x=b;
    //           0 or end-of-file means factorization only.
    int order;
    if (!(std::cin >> order) || order <= 0) {
        std::cerr << "输入错误：请先输入正整数矩阵阶数 n。\n";
        return 1;
    }

    const std::size_t n = static_cast<std::size_t>(order);
    Matrix a(n, std::vector<double>(n));
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            if (!(std::cin >> a[i][j])) {
                std::cerr << "输入错误：矩阵元素不足或格式不正确。\n";
                return 1;
            }
        }
    }

    try {
        LUFactorization lu = doolittlePartialPivot(std::move(a));

        std::cout << std::setprecision(10);
        std::cout << "置换向量 pi（1-based，(P*A) 的第 i 行来自 A 的第 pi[i] 行）：\n";
        for (std::size_t i = 0; i < n; ++i) {
            std::cout << lu.permutation[i] + 1 << (i + 1 == n ? '\n' : ' ');
        }
        printCompact(lu.compact);
        printFactors(lu.compact);

        int solveFlag = 0;
        if (std::cin >> solveFlag) {
            if (solveFlag != 0 && solveFlag != 1) {
                throw std::invalid_argument("矩阵后应输入 0（只分解）或 1（同时求解）");
            }
            if (solveFlag == 1) {
                std::vector<double> b(n);
                for (double& value : b) {
                    if (!(std::cin >> value)) {
                        throw std::invalid_argument("输入错误：右端向量 b 的元素不足");
                    }
                }
                const std::vector<double> x = solve(lu, b);
                std::cout << "解向量 x：\n";
                for (std::size_t i = 0; i < n; ++i) {
                    std::cout << x[i] << (i + 1 == n ? '\n' : ' ');
                }
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "计算失败：" << error.what() << '\n';
        return 1;
    }

    return 0;
}
