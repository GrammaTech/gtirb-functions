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
import collections
import gtirb
import gtirb_capstone.instructions
import capstone


class Function(object):
    """A function is a collection of code blocks, with information about
    what the function should be named and what entrace and exit blocks it has.
    """

    def __init__(
        self,
        uuid,
        entryBlocks=None,
        blocks=None,
        name_symbols=None,
        exitBlocks=None,
    ):
        """Construct a new function.

        :param uuid: The UUID of the function. This value must not be the UUID
        of any existing code block.
        :param entryBlocks: A set of code blocks that represent possible entry
        points into the function.
        :param blocks: A set of code blocks that represent all blocks in this
        function.
        :param name_symbols: A set of symbols that could represent the name of
        the function.
        :param exitBlocks: A set of code blocks that represent possible points
        where control could leave the function.
        """

        self._uuid = uuid
        self._entryBlocks = entryBlocks
        self._exit_blocks = exitBlocks
        self._blocks = blocks
        self._name_symbols = name_symbols

    @classmethod
    def build_functions(cls, module):
        """Given a module, generate all the functions accosicated with it."""

        symbols = collections.defaultdict(set)
        for symbol in module.symbols:
            symbols[symbol.referent].add(symbol)

        outgoing_edge_map = collections.defaultdict(list)
        for edge in module.ir.cfg:
            outgoing_edge_map[edge.source].append(edge)

        functions = []
        for uuid, entryBlocks in module.aux_data[
            "functionEntries"
        ].data.items():
            blocks = module.aux_data["functionBlocks"].data[uuid]
            exits = cls._get_exit_blocks(blocks, outgoing_edge_map)
            syms = [s for b in entryBlocks for s in symbols[b]]
            functions.append(
                Function(
                    uuid,
                    entryBlocks=entryBlocks,
                    blocks=blocks,
                    name_symbols=syms,
                    exitBlocks=exits,
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

    @classmethod
    def _get_exit_blocks(cls, blocks, outgoing_edge_map):
        exit_blocks = set()

        # A block is an exit block if it ends in a return or a tail
        # call.
        for b in blocks:
            # Does it end in a ret instruction?
            decoder = gtirb_capstone.instructions.GtirbInstructionDecoder(
                b.module.isa
            )
            ends_in_ret = False
            ends_in_indirect_jump = False
            for instruction in decoder.get_instructions(b):
                if instruction.id == capstone.x86.X86_INS_RET:
                    ends_in_ret = True
                    break
                elif (
                    capstone.x86.X86_GRP_JUMP in instruction.groups
                    and instruction.operands[0].type == capstone.x86.X86_OP_REG
                ):
                    ends_in_indirect_jump = True
                    break
            if ends_in_ret:
                exit_blocks.add(b)
                continue

            # if it ends in an indirect jump, and all the possible targets
            # ends up in the same function, then it is NOT an exit block
            edges = outgoing_edge_map[b]
            if ends_in_indirect_jump and all(
                e.label.direct or (e.target in blocks) for e in edges
            ):
                continue

            # Does it end in a tail call?
            if len(edges) != 1:
                continue
            for e in edges:
                if (
                    e.label.type == gtirb.Edge.Type.Branch
                    and not e.label.conditional
                    and e.target not in blocks
                ):
                    exit_blocks.add(b)

        return exit_blocks

    def get_exit_blocks(self):
        """Get the set of exit blocks out of this function."""

        if self._exit_blocks is None:
            blocks = self.get_all_blocks()
            edge_map = collections.defaultdict(list)
            for ir in {b.ir for b in blocks}:
                for edge in ir.cfg:
                    edge_map[edge.source].append(edge)

            self._exit_blocks = self._get_exit_blocks(blocks, edge_map)

        return self._exit_blocks

    def get_all_blocks(self):
        """Get all blocks in this function."""

        return self._blocks

    @property
    def uuid(self):
        """Gets the UUID for the function."""

        return self._uuid

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
