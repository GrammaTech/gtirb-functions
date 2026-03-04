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
import uuid
from typing import Iterable, List, Optional, Set, Tuple


class Function(object):
    """A function is a collection of code blocks, with information about
    what the function should be named and what entrace and exit blocks it has.
    """

    def __init__(
        self,
        uuid: uuid.UUID,
        entryBlocks: Optional[Iterable[gtirb.CodeBlock]] = None,
        blocks: Optional[Iterable[gtirb.CodeBlock]] = None,
        name_symbols: Optional[Iterable[gtirb.Symbol]] = None,
        exitBlocks: Optional[Iterable[gtirb.CodeBlock]] = None,
        canonical_name_symbol: Optional[gtirb.Symbol] = None,
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
        :param canonical_name_symbol: The symbol that should be considered
        the canonical name for the function.
        """

        self._uuid = uuid
        self._entryBlocks = set() if entryBlocks is None else set(entryBlocks)
        self._exit_blocks = None if exitBlocks is None else set(exitBlocks)
        self._blocks = set() if blocks is None else set(blocks)
        self._name_symbols = (
            list() if name_symbols is None else list(name_symbols)
        )
        self._canonical_name_symbol = canonical_name_symbol
        if (
            canonical_name_symbol
            and canonical_name_symbol not in self._name_symbols
        ):
            self._name_symbols.append(canonical_name_symbol)

    @classmethod
    def build_functions(cls, module: gtirb.Module) -> List["Function"]:
        """Given a module, generate all the functions associated with it."""

        symbols = collections.defaultdict(set)
        for symbol in module.symbols:
            symbols[symbol.referent].add(symbol)

        functions = []
        for uuid, entryBlocks in module.aux_data[
            "functionEntries"
        ].data.items():
            blocks = module.aux_data["functionBlocks"].data[uuid]
            name = module.aux_data["functionNames"].data.get(uuid)
            syms = [s for b in entryBlocks for s in symbols[b]]
            functions.append(
                Function(
                    uuid,
                    entryBlocks=entryBlocks,
                    blocks=blocks,
                    name_symbols=syms,
                    exitBlocks=None,
                    canonical_name_symbol=name,
                )
            )
        return functions

    def get_name(self) -> str:
        """Get the name of this function as a str."""

        if self._canonical_name_symbol:
            return self._canonical_name_symbol.name

        names = [s.name for s in self._name_symbols]
        if len(names) == 1:
            return names[0]
        elif len(names) >= 2:
            return "{} (a.k.a. {})".format(names[0], ",".join(names[1:]))
        else:
            return "<unknown>"

    def get_entry_blocks(self) -> Set[gtirb.CodeBlock]:
        """Get the set of entry blocks into this function."""

        return self._entryBlocks

    def get_exit_blocks(self) -> Set[gtirb.CodeBlock]:
        """Get the set of exit blocks out of this function."""

        if self._exit_blocks is None:
            calls = {gtirb.Edge.Type.Call, gtirb.Edge.Type.Syscall}
            returns = {gtirb.Edge.Type.Return, gtirb.Edge.Type.Sysret}
            blocks = self.get_all_blocks()
            self._exit_blocks = {
                edge.source
                for block in blocks
                for edge in block.outgoing_edges
                if isinstance(edge.source, gtirb.CodeBlock)
                and edge.label
                and (
                    edge.label.type in returns
                    or (
                        edge.label.type not in calls
                        and edge.target not in blocks
                    )
                )
            }

        return self._exit_blocks

    def get_all_blocks(self) -> Set[gtirb.CodeBlock]:
        """Get all blocks in this function."""

        return self._blocks

    @property
    def uuid(self) -> uuid.UUID:
        """Gets the UUID for the function."""

        return self._uuid

    @property
    def canonical_name_symbol(self) -> Optional[gtirb.Symbol]:
        """
        The canonical/primary name symbol for the function. May be None if the
        function has no associated symbols or if there are multiple symbols
        but none them are known to be the canonical symbol.
        """
        return self._canonical_name_symbol

    @property
    def name_symbols(self) -> List[gtirb.Symbol]:
        return self._name_symbols

    @property
    def names(self) -> List[str]:
        return [s.name for s in self.name_symbols]

    def __repr__(self) -> str:
        def block_sort_key(
            x: gtirb.CodeBlock,
        ) -> Tuple[Optional[gtirb.ByteInterval], int]:
            return (x.byte_interval, x.offset)

        return "[UUID={}, Name={}, Entry={}, Exit={}, All={}]".format(
            self._uuid,
            self.get_name(),
            sorted(list(self._entryBlocks), key=block_sort_key),
            sorted(list(self.get_exit_blocks()), key=block_sort_key),
            sorted(list(self._blocks), key=block_sort_key),
        )
