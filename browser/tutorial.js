/* Contextual tutorials never gate progression; returning players may disable. */
window.MGTutorial = {
  get(state){
    if(!state.showTips || state.finished) return null;
    if(state.stage===0){
      if(!state.moved) return ['MOVEMENT','Use WASD / left stick to move. The mission starts in captivity; nothing here is a training checklist.'];
      if(!state.looked) return ['FREE CAMERA','Move the mouse / right stick to examine your cell. Keep moving while you look.'];
      if(!state.crouched) return ['STEALTH STANCE','Press Ctrl / B to crouch. Slow, low movement is quieter and harder for guards to see.'];
      return ['ENVIRONMENT','The damaged east wall is different from its surroundings. Approach it, face it, press E / A.'];
    }
    if(state.stage===1) return ['RESCUE','You can move while listening. Approach Gray Fox and press E / A; his intelligence will remain in the log.'];
    if(state.stage===2) return ['ESCAPE','Open the second weak wall; it is not safe beyond. Use crouch, prone (Z / D-pad down) or cover (V).'];
    if(state.stage===3){
      if(!state.openedEquip) return ['EQUIPMENT','Hold Tab / LB for items, G / RB for weapons. Time slows while choosing; confiscated equipment is unavailable.'];
      return ['ENEMY AWARENESS','Red means guards have a confirmed alert. Yellow means they are investigating. Move around cover, not through their sightlines.'];
    }
    return null;
  },
  getHint(state){
    if(state.stage>=4) return '';
    const elapsed=Math.max(0,state.elapsedStage);
    if(elapsed<28)return '';
    const detail=elapsed>=75;
    if(state.stage===0)return detail?'The central patch on the east wall will break when you approach and interact.':'Listen for the hollow wall panel.';
    if(state.stage===1)return detail?'Gray Fox is waiting near the centre of the adjacent cell.':'A prisoner is nearby.';
    if(state.stage===2)return detail?'The repaired wall beside the service passage sounds different.':'A second wall separates the cells from a guard passage.';
    return detail?'Use the crates to break sightlines; head towards the marked exit at the east end.':'A crouched approach is quiet, but not invisible.';
  }
};
