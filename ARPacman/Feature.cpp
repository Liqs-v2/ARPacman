#include "Feature.h"

Feature feature;

Feature::Feature() {

}

void Feature::colorArrowTop() {
    feature.arrowUp = true;
    feature.arrowDown = false;
    feature.arrowLeft = false;
    feature.arrowRight = false;
}

void Feature::colorArrowBottom() {
    feature.arrowUp = false;
    feature.arrowDown = true;
    feature.arrowLeft = false;
    feature.arrowRight = false;
}

void Feature::colorArrowLeft() {
    feature.arrowUp = false;
    feature.arrowDown = false;
    feature.arrowLeft = true;
    feature.arrowRight = false;

}

void Feature::colorArrowRight() {
    feature.arrowUp = false;
    feature.arrowDown = false;
    feature.arrowLeft = false;
    feature.arrowRight = true;
}

void Feature::setBackground() {
    if (!feature.enableBackground) {
        feature.enableBackground = true;
    }
    else {
        feature.enableBackground = false;
    }
}

void Feature::setSound() {
    if (!feature.enableSound) {
        feature.enableSound = true;
    }
    else {
        feature.enableSound = false;
    }
}