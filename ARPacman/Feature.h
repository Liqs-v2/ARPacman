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
    void colorArrowTop();
    void colorArrowBottom();
    void colorArrowLeft();
    void colorArrowRight();
    void setBackground();
};
