#include <XiaoTuNetBox/DataQueue.hpp>
#include <gtest/gtest.h>

using namespace xiaotu::net;

TEST(DataQueue, create)
{
    DataQueue<double> *pQueue = new DataQueue<double>();
    EXPECT_EQ(pQueue->Size(), 0);
    EXPECT_TRUE(pQueue->Empty());
    EXPECT_FALSE(pQueue->Full());
    EXPECT_EQ(pQueue->Capacity(), 4);
    EXPECT_EQ(pQueue->Available(), 4);
    delete pQueue;
}

TEST(DataQueue, push_front_pop_front)
{
    DataQueue<double> *pQueue = new DataQueue<double>();

    for (int i = 0; i < 5; i++) {
        pQueue->PushFront(1.1 + i);
        EXPECT_EQ(pQueue->Size(), 1 + i);
    }
    EXPECT_EQ(pQueue->Size(), 5);

    double tmp;
    for (int i = 5; i > 0; i--) {
        pQueue->PopFront(tmp);
        EXPECT_EQ(pQueue->Size(), i - 1);
        EXPECT_EQ(tmp, 0.1 + i);
    }
    EXPECT_EQ(pQueue->Size(), 0);
    EXPECT_FALSE(pQueue->PopFront(tmp));

    delete pQueue;
}

TEST(DataQueue, push_back_pop_back)
{
    DataQueue<double> *pQueue = new DataQueue<double>();

    for (int i = 0; i < 5; i++) {
        pQueue->PushBack(1.1 + i);
        EXPECT_EQ(pQueue->Size(), 1 + i);
    }
    EXPECT_EQ(pQueue->Size(), 5);

    double tmp;
    for (int i = 5; i > 0; i--) {
        pQueue->PopBack(tmp);
        EXPECT_EQ(pQueue->Size(), i - 1);
        EXPECT_EQ(tmp, 0.1 + i);
    }
    EXPECT_EQ(pQueue->Size(), 0);
    EXPECT_FALSE(pQueue->PopBack(tmp));

    delete pQueue;
}

TEST(DataQueue, push_back_pop_front)
{
    DataQueue<double> *pQueue = new DataQueue<double>();

    double tmp;
    for (int i = 0; i < 13; i++) {
        EXPECT_TRUE(pQueue->PushBack(1.2 * i));
        EXPECT_EQ(pQueue->Size(), i+1);
    }

    EXPECT_EQ(pQueue->Size(), 13);
    EXPECT_EQ(pQueue->Capacity(), 16);
    pQueue->PeekFront(tmp);
    EXPECT_EQ(tmp, 0);
    pQueue->PeekBack(tmp);
    EXPECT_EQ(tmp, 1.2 * 12);

    for (int i = 0; i < 11; i++) {
        EXPECT_TRUE(pQueue->PopFront(tmp));
        EXPECT_EQ(tmp, 1.2 * i);
    }
    EXPECT_EQ(pQueue->Size(), 2);
    EXPECT_EQ(pQueue->Capacity(), 16);

    for (int i = 13; i < 20; i++) {
        EXPECT_TRUE(pQueue->PushBack(1.2 * i));
    }
    EXPECT_EQ(pQueue->Size(), 9);
    EXPECT_EQ(pQueue->Capacity(), 16);

    for (int i = 11; i < 17; i++) {
        EXPECT_TRUE(pQueue->PopFront(tmp));
        EXPECT_EQ(tmp, 1.2 * i);
    }
    EXPECT_EQ(pQueue->Size(), 3);
    EXPECT_EQ(pQueue->Capacity(), 16);

    pQueue->Rearrange();
    for (int i = 17; i < 19; i++) {
        EXPECT_TRUE(pQueue->PopFront(tmp));
        EXPECT_EQ(tmp, 1.2 * i);
    }
    EXPECT_EQ(pQueue->Size(), 1);

    for (int i = 20; i < 36; i++) {
        EXPECT_TRUE(pQueue->PushBack(1.2 * i));
    }
    EXPECT_EQ(pQueue->Size(), 17);
    EXPECT_EQ(pQueue->Capacity(), 32);

    pQueue->Rearrange();
    for (int i = 19; i < 35; i++) {
        EXPECT_TRUE(pQueue->PopFront(tmp));
        EXPECT_EQ(tmp, 1.2 * i);
    }
    EXPECT_EQ(pQueue->Size(), 1);

    EXPECT_TRUE(pQueue->PopFront(tmp));
    EXPECT_EQ(tmp, 1.2 * 35);
    EXPECT_TRUE(pQueue->Empty());

    delete pQueue;
}

TEST(DataQueue, push_back_peek_front)
{
    DataQueue<double> *pQueue = new DataQueue<double>();

    double tmp;
    for (int i = 0; i < 13; i++) {
        EXPECT_TRUE(pQueue->PushBack(1.2 * i));
        EXPECT_EQ(pQueue->Size(), i+1);
    }

    EXPECT_EQ(pQueue->Size(), 13);
    EXPECT_EQ(pQueue->Capacity(), 16);
    pQueue->PeekFront(tmp);
    EXPECT_EQ(tmp, 0);
    pQueue->PeekBack(tmp);
    EXPECT_EQ(tmp, 1.2 * 12);

    for (int i = 0; i < 11; i++) {
        EXPECT_TRUE(pQueue->PopFront(tmp));
        EXPECT_EQ(tmp, 1.2 * i);
    }
    EXPECT_EQ(pQueue->Size(), 2);
    EXPECT_EQ(pQueue->Capacity(), 16);

    for (int i = 13; i < 20; i++) {
        EXPECT_TRUE(pQueue->PushBack(1.2 * i));
    }
    EXPECT_EQ(pQueue->Size(), 9);
    EXPECT_EQ(pQueue->Capacity(), 16);

    pQueue->PeekFront(tmp);
    EXPECT_EQ(tmp, 1.2 * 11);

    for (int i = 0; i < 9; i++) {
        pQueue->PeekFront(tmp, i);
        EXPECT_EQ(tmp, 1.2 * (11 + i));
    }

    for (int i = 9; i < 16; i++) {
        EXPECT_FALSE(pQueue->PeekFront(tmp, i));
    }

    double tmp_array[9];
    bool suc = pQueue->PeekFront(tmp_array, 9, 0);
    EXPECT_TRUE(suc);

    for (int i = 0; i < 9; i++) {
        EXPECT_EQ(tmp_array[i], 1.2 * (11 + i));
    }

    suc = pQueue->PeekFront(tmp_array, 9, 1);
    EXPECT_FALSE(suc);

    suc = pQueue->PeekFront(tmp_array, 4, 3);
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(tmp_array[i], 1.2 * (11 + 3 + i));
    }

    delete pQueue;
}

TEST(DataQueue, push_front_pop_back)
{
    DataQueue<double> *pQueue = new DataQueue<double>();

    double tmp;
    for (int i = 0; i < 13; i++) {
        EXPECT_TRUE(pQueue->PushFront(1.2 * i));
        EXPECT_EQ(pQueue->Size(), i+1);
    }
    EXPECT_EQ(pQueue->Size(), 13);
    EXPECT_EQ(pQueue->Capacity(), 16);
    pQueue->PeekFront(tmp);
    EXPECT_EQ(tmp, 1.2 * 12);
    pQueue->PeekBack(tmp);
    EXPECT_EQ(tmp, 0);

    for (int i = 0; i < 11; i++) {
        EXPECT_TRUE(pQueue->PopBack(tmp));
        EXPECT_EQ(tmp, 1.2 * i);
    }
    EXPECT_EQ(pQueue->Size(), 2);
    EXPECT_EQ(pQueue->Capacity(), 16);

    for (int i = 13; i < 20; i++) {
        EXPECT_TRUE(pQueue->PushFront(1.2 * i));
    }
    EXPECT_EQ(pQueue->Size(), 9);
    EXPECT_EQ(pQueue->Capacity(), 16);

    for (int i = 11; i < 17; i++) {
        EXPECT_TRUE(pQueue->PopBack(tmp));
        EXPECT_EQ(tmp, 1.2 * i);
    }
    EXPECT_EQ(pQueue->Size(), 3);
    EXPECT_EQ(pQueue->Capacity(), 16);

    pQueue->Rearrange();
    for (int i = 17; i < 19; i++) {
        EXPECT_TRUE(pQueue->PopBack(tmp));
        EXPECT_EQ(tmp, 1.2 * i);
    }
    EXPECT_EQ(pQueue->Size(), 1);

    for (int i = 20; i < 36; i++) {
        EXPECT_TRUE(pQueue->PushFront(1.2 * i));
    }
    EXPECT_EQ(pQueue->Size(), 17);
    EXPECT_EQ(pQueue->Capacity(), 32);

    pQueue->Rearrange();
    for (int i = 19; i < 35; i++) {
        EXPECT_TRUE(pQueue->PopBack(tmp));
        EXPECT_EQ(tmp, 1.2 * i);
    }
    EXPECT_EQ(pQueue->Size(), 1);

    EXPECT_TRUE(pQueue->PopBack(tmp));
    EXPECT_EQ(tmp, 1.2 * 35);
    EXPECT_TRUE(pQueue->Empty());

    delete pQueue;
}

TEST(DataQueue, push_nback_pop_nfront)
{
    DataQueue<double> *pQueue = new DataQueue<double>();
    double buf[5];
    for (int i = 0; i < 5; i++)
        buf[i] = 1.1 + i;

    pQueue->PushBack(buf, 5);
    EXPECT_EQ(pQueue->Size(), 5);
 
    double pop_buf[50];
    pQueue->PopFront(pop_buf, 5);
    for (int i = 0; i < 5; i++)
        EXPECT_EQ(pop_buf[i], buf[i]);
    EXPECT_EQ(pQueue->Size(), 0);

    pQueue->PushBack(buf, 5);
    pQueue->PushBack(buf, 5);
    EXPECT_EQ(pQueue->Size(), 10);
 
    pQueue->PopFront(pop_buf, 8);
    for (int i = 0; i < 5; i++)
        EXPECT_EQ(pop_buf[i], buf[i]);
    for (int i = 0; i < 3; i++)
        EXPECT_EQ(pop_buf[5+i], buf[i]);
    EXPECT_EQ(pQueue->Size(), 2);

    pQueue->PushBack(buf, 5);
    EXPECT_EQ(pQueue->Size(), 7);

    pQueue->PopFront(pop_buf, 2);
    EXPECT_EQ(pop_buf[0], buf[3]);
    EXPECT_EQ(pop_buf[1], buf[4]);

    pQueue->PopFront(pop_buf, 5);
    for (int i = 0; i < 5; i++)
        EXPECT_EQ(pop_buf[i], buf[i]);
    EXPECT_EQ(pQueue->Size(), 0);

    delete pQueue;
}

TEST(DataQueue, push_nback_drop_nfront)
{
    DataQueue<double> *pQueue = new DataQueue<double>();
    double buf[5];
    for (int i = 0; i < 5; i++)
        buf[i] = 1.1 + i;

    pQueue->PushBack(buf, 5);
    EXPECT_EQ(pQueue->Size(), 5);
    EXPECT_EQ(pQueue->Capacity(), 10);

    pQueue->DropFront(5);
    EXPECT_EQ(pQueue->Size(), 0);
    EXPECT_EQ(pQueue->Capacity(), 10);
    EXPECT_FALSE(pQueue->Folded());

    double pop_buf[50];
    pQueue->PushBack(buf, 5);
    pQueue->PushBack(buf, 5);
    EXPECT_EQ(pQueue->Size(), 10);
    EXPECT_EQ(pQueue->Capacity(), 10);
 
    pQueue->DropFront(8);
    EXPECT_EQ(pQueue->Size(), 2);
    EXPECT_TRUE(pQueue->Folded());
    EXPECT_EQ(pQueue->FoldedHead(), 2);
    EXPECT_EQ(pQueue->FoldedTail(), 0);


    double tmp;
    pQueue->PopBack(tmp);
    EXPECT_EQ(tmp, buf[4]);
    EXPECT_FALSE(pQueue->Folded());

    pQueue->PushBack(buf, 5);
    EXPECT_EQ(pQueue->Size(), 6);
    EXPECT_TRUE(pQueue->Folded());
    EXPECT_EQ(pQueue->FoldedHead(), 2);
    EXPECT_EQ(pQueue->FoldedTail(), 4);

    pQueue->DropFront(1);
    pQueue->PopFront(pop_buf, 5);
    for (int i = 0; i < 5; i++)
        EXPECT_EQ(pop_buf[i], buf[i]);
    EXPECT_EQ(pQueue->Size(), 0);

    delete pQueue;
}

TEST(DataQueue, push_nback_pop_back)
{
    DataQueue<double> *pQueue = new DataQueue<double>();
    double buf[5];
    for (int i = 0; i < 5; i++)
        buf[i] = 1.1 + i;

    pQueue->PushBack(buf, 5);
    EXPECT_EQ(pQueue->Size(), 5);
    
    double tmp;
    for (int i = 5; i > 0; i--) {
        pQueue->PopBack(tmp);
        EXPECT_EQ(pQueue->Size(), i - 1);
        EXPECT_EQ(tmp, 0.1 + i);
    }
    EXPECT_EQ(pQueue->Size(), 0);
    EXPECT_FALSE(pQueue->PopBack(tmp));

    pQueue->PushBack(buf, 5);
    EXPECT_EQ(pQueue->Size(), 5);
    for (int i = 5; i > 2; i--) {
        pQueue->PopBack(tmp);
        EXPECT_EQ(pQueue->Size(), i - 1);
        EXPECT_EQ(tmp, 0.1 + i);
    }
    EXPECT_EQ(pQueue->Size(), 2);

    pQueue->PushBack(buf, 2);
    EXPECT_EQ(pQueue->Size(), 4);
    for (int i = 2; i > 0; i--) {
        pQueue->PopBack(tmp);
        EXPECT_EQ(pQueue->Size(), i - 1 + 2);
        EXPECT_EQ(tmp, 0.1 + i);
    }
    EXPECT_EQ(pQueue->Size(), 2);
    for (int i = 2; i > 0; i--) {
        pQueue->PopBack(tmp);
        EXPECT_EQ(pQueue->Size(), i - 1);
        EXPECT_EQ(tmp, 0.1 + i);
    }
    EXPECT_EQ(pQueue->Size(), 0);

    delete pQueue;
}

