//
// Created by scros on 11/15/25.
//

export module Engine.AnimationChannel;

export enum class AnimationChannelType
{
    Translation,
    Rotation,
    Scale,
};

export struct AnimationChannel
{
    int sampler{-1};
    int node{-1};
    AnimationChannelType type{};
};
