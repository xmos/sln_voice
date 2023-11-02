#!/usr/bin/env python3
# Copyright 2022-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
# XMOS Public License: Version 1

import argparse

PIPELINE_BRICK_LENGTH_MS = 15
PIPELINE_BRICK_LENGTH_S = PIPELINE_BRICK_LENGTH_MS / 1000.0

def events_overlap(unscored, truth):
    def is_between(q, x, y):
        return q >= x and q <= y

    # allow some slack in the truth start and end
    truth_start =  truth['start'] - PIPELINE_BRICK_LENGTH_S
    truth_end =  truth['end'] + PIPELINE_BRICK_LENGTH_S

    if is_between(unscored['start'], truth_start, truth_end) or \
       is_between(unscored['end'], truth_start, truth_end) or \
       is_between(truth_start, unscored['start'], unscored['end']) or \
       is_between(truth_end, unscored['start'], unscored['end']) :
        return True

    return False

def process(truth_track, label_track, log):
    truth_events = []
    unscored_events = []

    # load truth track
    with open(truth_track, "r") as fd:
        for line in fd:
            fields = line.strip().split("\t")
            truth_events.append({
                'start': float(fields[0]),
                'end': float(fields[1]),   
                'label': fields[2]
            })

    # load track to score
    with open(label_track, "r") as fd:
        for line in fd:
            fields = line.strip().split("\t")
            unscored_events.append({
                'start': float(fields[0]),
                'end': float(fields[1]),   
                'label': fields[2]
            })

    substitutions = []
    insertions = []
    deletions = []
    correct = []

    for i_unscored, unscored_event in enumerate(unscored_events):
        #print(unscored_event)
        not_scored = True
        for i_truth, truth_event in enumerate(truth_events):
            overlap = events_overlap(truth_event, unscored_event)
            #print("  ", overlap, truth_event)
            if overlap:
                if truth_event['label'] == unscored_event['label']:
                    # event matches expected truth => correct
                    correct.append(unscored_event)
                    del truth_events[i_truth]
                    not_scored = False
                    continue
                else:
                    # event does not match expected truth => substitution
                    substitutions.append(unscored_event)
                    del truth_events[i_truth]
                    not_scored = False
                    continue


        if not_scored:
            # did not find truth for this event => insertion
            insertions.append(unscored_event)

    # any remaining truth events => deletions
    deletions = truth_events

    # print report log
    with open(log, "w") as fd:
        def print_events(events):
            for ev in events:
                print(ev, file=fd)

        print("****************************", file=fd)
        print("CORRECT:", len(correct), file=fd)
        print("****************************", file=fd)
        print_events(correct)
        print("****************************", file=fd)
        print("SUBSTITUTIONS:", len(substitutions), file=fd)
        print("****************************", file=fd)
        print_events(substitutions)
        print("****************************", file=fd)
        print("INSERTIONS:", len(insertions), file=fd)
        print("****************************", file=fd)
        print_events(insertions)
        print("****************************", file=fd)
        print("DELETIONS:", len(deletions), file=fd)
        print("****************************", file=fd)
        print_events(deletions)

        print(file=fd)
        wer = (len(substitutions) + len(deletions) + len(insertions)) / \
            (len(substitutions) + len(deletions) + len(insertions) + len(correct))
        print(f"WER: {wer}", file=fd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Label Track Scorer')
    parser.add_argument('--truth_track', help='Truth track file')
    parser.add_argument('--label_track', help='Label track file')
    parser.add_argument('--log', help='Scoring log file')
    args = parser.parse_args()

    process(args.truth_track, args.label_track, args.log)