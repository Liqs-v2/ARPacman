#pragma once

class Feature
{
public:
    Feature();
    bool arrowUp;
    bool arrowDown;
    bool arrowLeft;
    bool arrowRight;
    bool enableBackground;
    bool enableSound;
    void colorArrowTop();
    void colorArrowBottom();
    void colorArrowLeft();
    void colorArrowRight();
    void setBackground();
    void setSound();
};
