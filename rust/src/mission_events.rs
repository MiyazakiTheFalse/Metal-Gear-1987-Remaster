//! Deterministic, engine-neutral mission event state.
//! Gameplay effects are data; Unreal is responsible for animation/audio/camera.
//! A successful event is committed once, never partially, then emits cues.
//! No J2ME source event is assumed verified here.
use std::collections::BTreeSet;

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Effect {
    SetFlag(String),
    ClearFlag(String),
    PresentationCue(String),
}

#[derive(Clone, Debug)]
pub struct Rule {
    pub id: String,
    pub trigger: String,
    pub requires_all: Vec<String>,
    pub forbids_any: Vec<String>,
    pub effects: Vec<Effect>,
    pub once: bool,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum EventError {
    DuplicateRule(String),
    InvalidRule(String),
    UnknownEventInSave(String),
    UnsupportedSaveVersion(u32),
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Activation {
    pub event_id: String,
    pub presentation_cues: Vec<String>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct MissionSnapshot {
    pub version: u32,
    pub flags: BTreeSet<String>,
    pub fired_once: BTreeSet<String>,
}

pub struct EventRuntime {
    rules: Vec<Rule>,
    flags: BTreeSet<String>,
    fired_once: BTreeSet<String>,
}

impl EventRuntime {
    pub fn new(mut rules: Vec<Rule>) -> Result<Self, EventError> {
        rules.sort_by(|a, b| a.id.cmp(&b.id));
        let mut known = BTreeSet::new();
        for rule in &rules {
            if rule.id.is_empty() || rule.trigger.is_empty() {
                return Err(EventError::InvalidRule(rule.id.clone()));
            }
            if !known.insert(rule.id.clone()) {
                return Err(EventError::DuplicateRule(rule.id.clone()));
            }
            let mut grants = BTreeSet::new();
            let mut clears = BTreeSet::new();
            for effect in &rule.effects {
                match effect {
                    Effect::SetFlag(flag) => { grants.insert(flag.as_str()); },
                    Effect::ClearFlag(flag) => { clears.insert(flag.as_str()); },
                    Effect::PresentationCue(_) => {}
                }
            }
            if grants.iter().any(|key| key.is_empty() || clears.contains(key))
               || clears.contains("") || rule.requires_all.iter().any(|s| s.is_empty())
               || rule.forbids_any.iter().any(|s| s.is_empty())
               || rule.requires_all.iter().any(|s| rule.forbids_any.contains(s)) {
                return Err(EventError::InvalidRule(rule.id.clone()));
            }
        }
        Ok(Self { rules, flags: BTreeSet::new(), fired_once: BTreeSet::new() })
    }

    pub fn has_flag(&self, flag: &str) -> bool { self.flags.contains(flag) }
    pub fn has_fired(&self, event_id: &str) -> bool { self.fired_once.contains(event_id) }

    /// Evaluate every rule against the SAME pre-event snapshot.
    /// This avoids hidden coupling to the alphabetical order of rules.
    /// Effects are applied only after the eligible batch is resolved.
    pub fn emit(&mut self, trigger: &str) -> Vec<Activation> {
        let eligible: Vec<&Rule> = self.rules.iter().filter(|rule| {
            rule.trigger == trigger
                && (!rule.once || !self.fired_once.contains(&rule.id))
                && rule.requires_all.iter().all(|f| self.flags.contains(f))
                && rule.forbids_any.iter().all(|f| !self.flags.contains(f))
        }).collect();

        let mut result = Vec::new();
        let mut set = BTreeSet::new();
        let mut clear = BTreeSet::new();
        for rule in eligible {
            let mut cues = Vec::new();
            for effect in &rule.effects {
                match effect {
                    Effect::SetFlag(flag) => { set.insert(flag.clone()); },
                    Effect::ClearFlag(flag) => { clear.insert(flag.clone()); },
                    Effect::PresentationCue(cue) => cues.push(cue.clone()),
                }
            }
            if rule.once {
                self.fired_once.insert(rule.id.clone());
            }
            result.push(Activation { event_id: rule.id.clone(), presentation_cues: cues });
        }
        // If two independent events conflict, clearing dominates conservatively.
        // A production authoring validator should prohibit conflicting rules.
        for flag in set { self.flags.insert(flag); }
        for flag in clear { self.flags.remove(&flag); }
        result
    }

    pub fn snapshot(&self) -> MissionSnapshot {
        MissionSnapshot { version: 1, flags: self.flags.clone(), fired_once: self.fired_once.clone() }
    }

    /// Restore ONLY durable state; never replay cinematics/audio on load.
    pub fn restore(&mut self, save: MissionSnapshot) -> Result<(), EventError> {
        if save.version != 1 {
            return Err(EventError::UnsupportedSaveVersion(save.version));
        }
        for id in &save.fired_once {
            if !self.rules.iter().any(|r| &r.id == id && r.once) {
                return Err(EventError::UnknownEventInSave(id.clone()));
            }
        }
        self.flags = save.flags;
        self.fired_once = save.fired_once;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    fn rule() -> Rule {
        Rule {
            id: "rescue.gray_fox".into(), trigger: "interact.gray_fox".into(),
            requires_all: vec!["door.cell.open".into()],
            forbids_any: vec!["mission.gray_fox.rescued".into()],
            effects: vec![
                Effect::SetFlag("mission.gray_fox.rescued".into()),
                Effect::PresentationCue("dialogue.gray_fox.rescue".into())
            ], once: true
        }
    }

    #[test]
    fn prerequisites_block_and_one_shot_does_not_repeat() {
        let mut e = EventRuntime::new(vec![rule()]).unwrap();
        assert!(e.emit("interact.gray_fox").is_empty());
        let mut save = e.snapshot();
        save.flags.insert("door.cell.open".into());
        e.restore(save).unwrap();
        assert_eq!(e.emit("interact.gray_fox").len(), 1);
        assert!(e.has_flag("mission.gray_fox.rescued"));
        assert_eq!(e.emit("interact.gray_fox").len(), 0);
    }

    #[test]
    fn snapshot_restores_without_replaying_presentation() {
        let mut e = EventRuntime::new(vec![rule()]).unwrap();
        let mut save = e.snapshot();
        save.flags.insert("door.cell.open".into());
        e.restore(save).unwrap();
        let activated = e.emit("interact.gray_fox");
        assert_eq!(activated[0].presentation_cues, vec!["dialogue.gray_fox.rescue"]);
        let snapshot = e.snapshot();
        let mut loaded = EventRuntime::new(vec![rule()]).unwrap();
        loaded.restore(snapshot).unwrap();
        assert!(loaded.emit("interact.gray_fox").is_empty());
        assert!(loaded.has_fired("rescue.gray_fox"));
    }

    #[test]
    fn same_tick_dependency_does_not_leak_between_rules() {
        let grant = Rule {
            id: "a".into(), trigger: "zone.entry".into(),
            requires_all: vec![], forbids_any: vec![],
            effects: vec![Effect::SetFlag("gate".into())], once: true
        };
        let later = Rule {
            id: "b".into(), trigger: "zone.entry".into(),
            requires_all: vec!["gate".into()], forbids_any: vec![],
            effects: vec![Effect::PresentationCue("radio".into())], once: true
        };
        let mut runtime = EventRuntime::new(vec![later,grant]).unwrap();
        assert_eq!(runtime.emit("zone.entry").len(), 1);
        assert_eq!(runtime.emit("zone.entry").len(), 1);
    }

    #[test]
    fn rejects_duplicates_and_conflicting_effects() {
        assert_eq!(EventRuntime::new(vec![rule(), rule()]).err(),
                   Some(EventError::DuplicateRule("rescue.gray_fox".into())));
        let mut invalid = rule();
        invalid.effects.push(Effect::ClearFlag("mission.gray_fox.rescued".into()));
        assert!(matches!(EventRuntime::new(vec![invalid]), Err(EventError::InvalidRule(_))));
    }

    #[test]
    fn unknown_event_save_is_rejected_without_mutating_state() {
        let mut runtime = EventRuntime::new(vec![rule()]).unwrap();
        let mut invalid = runtime.snapshot();
        invalid.fired_once.insert("nonexistent".into());
        assert!(matches!(runtime.restore(invalid), Err(EventError::UnknownEventInSave(_))));
        assert!(!runtime.has_fired("nonexistent"));
    }
}
