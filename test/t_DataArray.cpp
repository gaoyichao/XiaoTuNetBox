#include <XiaoTuNetBox/DataArray.hpp>
#include <gtest/gtest.h>
#include <iostream>

using namespace xiaotu::net;

TEST(DataArray, init)
{
    DataArray<double> *pArray = new DataArray<double>();
    EXPECT_EQ(pArray->Size(), 0);
    EXPECT_EQ(pArray->Capacity(), 0);
    EXPECT_TRUE(pArray->Empty());
    EXPECT_TRUE(pArray->Full());
    delete pArray;

    pArray = new DataArray<double>(4);
    EXPECT_EQ(pArray->Size(), 4);
    delete pArray;

    // 初始赋值
    double value[9];
    for (int i = 0; i < 9; i++)
        value[i] = 3.14 * i;
    pArray = new DataArray<double>(value, 9);
    EXPECT_EQ(pArray->Size(), 9);
    for (int i = 0; i < 9; i++)
        EXPECT_EQ((*pArray)[i], value[i]);
    value[5] = 101.1;
    EXPECT_NE((*pArray)[5], value[5]);

    // 拷贝构造
    DataArray<double> array = *pArray;
    EXPECT_EQ(pArray->Size(), 9);
    for (int i = 0; i < 9; i++)
        EXPECT_EQ((*pArray)[i], array[i]);
    array[5] = 101.1;
    EXPECT_NE((*pArray)[5], array[5]);
    delete pArray;

    // 类似Eigen的初始化
    pArray = new DataArray<double>();
    *pArray << 0.0, 1.0, 2.0, 3.0, 4.0, 5.0;
    EXPECT_EQ(pArray->Size(), 6);
    delete pArray;
}

TEST(DataArray, Resize)
{
    double datas[4];
    datas[0] = 1.2;
    datas[1] = 3.2;
    datas[2] = 5.2;
    datas[3] = 1.332;
    DataArray<double> array(datas, 4);
    EXPECT_EQ(array.Size(), 4);

    array.AdjustCapacity(10);
    EXPECT_EQ(array.Size(), 4);
    EXPECT_EQ(array.Capacity(), 10);
    EXPECT_EQ(array.Available(), 6);
    for (int i = 0; i < 4; i++)
        EXPECT_EQ(array[i], datas[i]);

    EXPECT_EQ(array.AdjustCapacity(3), 1);
    EXPECT_EQ(array.Size(), 4);
    EXPECT_EQ(array.Capacity(), 10);

    EXPECT_EQ(array.AdjustCapacity(10), 0);
    EXPECT_EQ(array.Size(), 4);

    EXPECT_EQ(array.Resize(3), 0);
    EXPECT_EQ(array.Size(), 3);
    EXPECT_EQ(array.Capacity(), 10);

    array.Clear();
    EXPECT_EQ(array.Size(), 0);

}

TEST(DataArray, Find)
{
    double datas[4];
    datas[0] = 1.2;
    datas[1] = 3.2;
    datas[2] = 5.2;
    datas[3] = 1.332;
    DataArray<double> array(datas, 4);
    EXPECT_EQ(array.Size(), 4);

    double *ptr = array.Find(datas[3]);
    int idx = array.FindIdx(datas[3]);
    EXPECT_EQ(ptr, &(array[idx]));
    EXPECT_EQ(idx, 3);

    EXPECT_TRUE(array.Find(1.2));
    EXPECT_FALSE(array.Find(3.14));
}

TEST(DataArray, Insert)
{
    double datas[4];
    datas[0] = 1.2;
    datas[1] = 3.2;
    datas[2] = 5.2;
    datas[3] = 1.332;
    DataArray<double> array;
    EXPECT_EQ(array.Size(), 0);
    for (int i = 0; i < 4; i++)
        array.Insert(i, datas[i]);
    EXPECT_EQ(array.Size(), 4);

    double pi = 3.1415926;
    array.Insert(2, pi);
    EXPECT_EQ(array.Size(), 5);
    EXPECT_EQ(array[0], datas[0]);
    EXPECT_EQ(array[1], datas[1]);
    EXPECT_EQ(array[2], pi);
    EXPECT_EQ(array[3], datas[2]);
    EXPECT_EQ(array[4], datas[3]);

    array.Insert(2, datas, 4);
    EXPECT_EQ(array.Size(), 9);
    EXPECT_EQ(array[0], datas[0]);
    EXPECT_EQ(array[1], datas[1]);
    for (int i = 0; i < 4; i++)
        EXPECT_EQ(array[2 + i], datas[i]);

    EXPECT_EQ(array[6], 3.1415926);
    EXPECT_EQ(array[7], datas[2]);
    EXPECT_EQ(array[8], datas[3]);

    EXPECT_EQ(array.Insert(10, 10), 1);

    for (int i = 9; i < 64; i++)
        array.Insert(i, i);

    for (int i = 9; i < 64; i++)
        EXPECT_EQ(array[i], i);
}

TEST(DataArray, Remove)
{
    double datas[4];
    datas[0] = 1.2;
    datas[1] = 3.2;
    datas[2] = 5.2;
    datas[3] = 1.332;
    DataArray<double> array(datas, 4);
    EXPECT_EQ(array.Size(), 4);

    double pi = 3.1415926;
    array.Insert(2, pi);
    EXPECT_EQ(array.Size(), 5);
    EXPECT_EQ(array[2], pi);

    array.Remove(2);
    EXPECT_EQ(array.Size(), 4);
    EXPECT_EQ(array[0], datas[0]);
    EXPECT_EQ(array[1], datas[1]);
    EXPECT_EQ(array[2], datas[2]);
    EXPECT_EQ(array[3], datas[3]);

    array.Insert(2, datas, 4);
    EXPECT_EQ(array.Size(), 8);

    array.Remove(2, 2+4);
    EXPECT_EQ(array.Size(), 4);
    EXPECT_EQ(array[0], datas[0]);
    EXPECT_EQ(array[1], datas[1]);
    EXPECT_EQ(array[2], datas[2]);
    EXPECT_EQ(array[3], datas[3]);

    array.Insert(2, datas, 4);
    EXPECT_EQ(array.Size(), 8);
    EXPECT_EQ(array.Remove(6, 2), 1);
    EXPECT_EQ(array.Remove(8), 2);

    array.Remove(1, 8);
    array.Remove(0);
    EXPECT_EQ(array.Size(), 0);
}

TEST(DataArray, Swap)
{
    double datas[4];
    datas[0] = 1.2;
    datas[1] = 3.2;
    datas[2] = 5.2;
    datas[3] = 1.332;
    DataArray<double> array(datas, 4);
    EXPECT_EQ(array.Size(), 4);

    EXPECT_EQ(1, array.Swap(0, 5));
    EXPECT_EQ(0, array.Swap(1, 0));

    EXPECT_EQ(array[0], datas[1]);
    EXPECT_EQ(array[1], datas[0]);
}


TEST(DataArray, stack)
{
    DataArray<double> *pArray = new DataArray<double>();
    EXPECT_EQ(pArray->Size(), 0);
    EXPECT_EQ(pArray->Capacity(), 0);
    EXPECT_TRUE(pArray->Empty());
    EXPECT_TRUE(pArray->Full());

    for (int i = 0; i < 5; i++) {
        pArray->Push(1.1 + i);
        EXPECT_EQ(pArray->Size(), 1 + i);
        EXPECT_EQ((*pArray)[i], 1.1 + i);
    }
    EXPECT_EQ(pArray->Size(), 5);

    double tmp;
    for (int i = 5; i > 0; i--) {
        pArray->Pop(tmp);
        EXPECT_EQ(pArray->Size(), i - 1);
        EXPECT_EQ(tmp, 0.1 + i);
    }
    EXPECT_EQ(pArray->Size(), 0);
    EXPECT_EQ(pArray->Pop(tmp), 1);

    delete pArray;
}



