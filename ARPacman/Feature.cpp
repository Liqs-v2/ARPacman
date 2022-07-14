#include "Feature.h"

Feature::Feature() {

}

void Feature::colorArrowTop() {
    arrowUp = true;
    arrowDown = false;
    arrowLeft = false;
    arrowRight = false;
}

void Feature::colorArrowBottom() {
    arrowUp = false;
    arrowDown = true;
    arrowLeft = false;
    arrowRight = false;
}

void Feature::colorArrowLeft() {
    arrowUp = false;
    arrowDown = false;
    arrowLeft = true;
    arrowRight = false;

}

void Feature::colorArrowRight() {
    arrowUp = false;
    arrowDown = false;
    arrowLeft = false;
    arrowRight = true;
}

void Feature::setBackground() {
    if (!enableBackground) {
        enableBackground = true;
    }
    else {
        enableBackground = false;
    }
}