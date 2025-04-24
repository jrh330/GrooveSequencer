#include "GridSequencerComponent.h"
#include "../GrooveSequencerLookAndFeel.h"

void GridCell::updateColour(const GrooveSequencerLookAndFeel& lf) 
{
    if (!active) {
        colour = lf.getGridCellInactiveColour();
        return;
    }
    
    // Base color depends on accent level
    switch (accent) {
        case 0: colour = lf.getGridCellActiveColour(); break;
        case 1: colour = lf.getGridCellAccentColour(); break;
        case 2: colour = lf.getGridCellStrongAccentColour(); break;
        default: colour = lf.getGridCellActiveColour(); break;
    }
    
    // Adjust alpha based on velocity
    float alpha = 0.3f + (velocity / 127.0f) * 0.7f;
    colour = colour.withAlpha(alpha);
    
    // Add staccato indicator
    if (isStaccato) {
        colour = colour.brighter(0.2f);
    }
} 