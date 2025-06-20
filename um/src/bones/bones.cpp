#include "bones.hpp"

BoneConnection boneConnections[] = {
    {6, 5},                                // head to neck
    {5, 4},                                // neck to spine
    {4, 0},                                // spine to hip
    {4, 8},                                // spine to left shoulder
    {8, 9},                                // left shoulder to left arm
    {9, 11},                               // arm to hand
    {4, 13},  {13, 14}, {14, 16}, {4, 2},  // spine to spine_1
    {0, 22},                               // hip to left_hip
    {0, 25},                               // hip to right_hip
    {22, 23},                              // left_hip to left_knee
    {23, 24},                              // left knee to left foot
    {25, 26},                              // right_hip to right_knee
    {26, 27}                               // right knee to right foot
};

const int boneConnectionsCount = sizeof(boneConnections) / sizeof(BoneConnection);
