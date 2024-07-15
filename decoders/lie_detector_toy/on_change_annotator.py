##
## Copyright (C) 2024 Michal Matias <mich4l.matias@gmail.com>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, see <http://www.gnu.org/licenses/>.
##

class OnChangeAnnotator:
    def __init__(self, put_fn, out_ann, ann_fn):
        self.last_state = None
        self.last_state_start_sample = 0
        self.last_start_sample = 0

        # external functions
        self.put = put_fn
        self.out_ann = out_ann
        self.ann = ann_fn

    def start(self, start_samplenum):
        self.last_state_start_sample = start_samplenum
        self.last_start_sample = start_samplenum

    def handle_state(self, state: str, start_sample: int):
        if self.last_state is None:
            self.last_state = state
        elif state != self.last_state:
            self.put(self.last_state_start_sample, self.last_start_sample, self.out_ann, self.ann(self.last_state))
            self.last_state_start_sample = self.last_start_sample
            self.last_state = state
        self.last_start_sample = start_sample

    # if an error occurs, break the continuity
    def disrupt(self, start_sample: int):
            self.last_state_start_sample = start_sample
            self.last_state = None
