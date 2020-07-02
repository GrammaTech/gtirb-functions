#
# Copyright (C) 2020 GrammaTech, Inc.
#
# This code is licensed under the MIT license. See the LICENSE file in
# the project root for license terms.
#
# This project is sponsored by the Office of Naval Research, One Liberty
# Center, 875 N. Randolph Street, Arlington, VA 22203 under contract #
# N68335-17-C-0700.  The content of the information does not necessarily
# reflect the position or policy of the Government and no official
# endorsement should be inferred.
#
from gtirb import Edge


class Function(object):
    """A function is a collection of code blocks, with information about
    what the function should be named and what entrace and exit blocks it has.
    """

    def __init__(self, uuid, entryBlocks=None, blocks=None, name_symbols=None):
        """Construct a new function.

        :param uuid: The UUID of the function. This value must not be the UUID
        of any existing code block.
        :param entryBlocks: A set of code blocks that represent possible entry
        points into the function.
        :param blocks: A set of code blocks that represent all blocks in this
        function.
        :param name_symbols: A set of symbols that could represent the name of
        the function.
        """

        self._uuid = uuid
        self._entryBlocks = entryBlocks
        self._exit_blocks = None
        self._blocks = blocks
        self._name_symbols = name_symbols

    @classmethod
    def build_functions(cls, module):
        """Given a module, generate all the functions accosicated with it."""

        functions = []
        for uuid, entryBlocks in module.aux_data[
            "functionEntries"
        ].data.items():
            entryBlocksUUID = set([e.uuid for e in entryBlocks])
            blocks = module.aux_data["functionBlocks"].data[uuid]
            syms = [
                x
                for x in filter(
                    lambda s: s.referent
                    and s.referent.uuid in entryBlocksUUID,
                    module.symbols,
                )
            ]
            functions.append(
                Function(
                    uuid,
                    entryBlocks=entryBlocks,
                    blocks=blocks,
                    name_symbols=syms,
                )
            )
        return functions

    def get_name(self):
        """Get the name of this function as a str."""

        names = [s.name for s in self._name_symbols]
        if len(names) == 1:
            return names[0]
        elif len(names) > 2:
            return "{} (a.k.a. {}".format(names[0], ",".join(names[1:]))
        else:
            return "<unknown>"

    def get_entry_blocks(self):
        """Get the set of entry blocks into this function."""

        return self._entryBlocks

    def get_exit_blocks(self):
        """Get the set of exit blocks out of this function."""

        if self._exit_blocks is None:
            self._exit_blocks = set()
            for b in self.get_all_blocks():
                for e in b.outgoing_edges:
                    # TODO Handle tail calls (jmp)
                    if e.label.type == Edge.Type.Return:
                        self._exit_blocks.add(b)

        return self._exit_blocks

    def get_all_blocks(self):
        """Get all blocks in this function."""

        return self._blocks

    def __repr__(self):
        def block_addr(x):
            return x.byte_interval.address + x.offset

        return "[UUID={}, Name={}, Entry={}, Exit={}, All={}]".format(
            self._uuid,
            self.get_name(),
            sorted(list(self._entryBlocks), key=block_addr),
            sorted(list(self.get_exit_blocks()), key=block_addr),
            sorted(list(self._blocks), key=block_addr),
        )
